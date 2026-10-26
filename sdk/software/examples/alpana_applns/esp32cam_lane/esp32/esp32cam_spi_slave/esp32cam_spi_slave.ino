/* ============================================================================
 * esp32cam_spi_slave.ino v4 : AI-Thinker ESP32-CAM -> ALPANA_SOC over SPI
 *                             320x240 JPEG video + 80x60 lane image, pipelined
 *
 *  v4: the camera now runs in COLOUR (no sensor grayscale effect) so that
 *  yellow paint can be told apart from grey concrete. The 80x60 image for
 *  ALPANA's lane detection is a "lane feature" image:
 *      feature = luma + YEL_GAIN * max(0, yellowness - YEL_T)
 *      yellowness = min(R,G) - B - 2*max(0, R-G-20)   (yellow >> wood/orange)
 *  computed from a 160x120 decode and pooled with (max+mean)/2, so thin and
 *  faded lines survive the downscale. White paint stays bright as before.
 *  Video: JPEG quality raised (HW_JPEG_QUALITY 12). The video is colour now;
 *  run the viewer with --gray if you want it black & white.
 *
 *  ESP32-CAM is the SPI SLAVE (mode 3). ALPANA_SOC (SPI master) pulls frames.
 *    GPIO14 <- SCLK   (Nexys JB3)
 *    GPIO15 -> MISO   (Nexys JB2)
 *    GPIO2  <- CS     (Nexys JD2, ALPANA GPIO25)
 *    GPIO12 -> HS     (Nexys JD1, ALPANA GPIO24)  high = transaction ready
 *    GND    -- GND
 *
 *  Link protocol v2: EVERY transaction is exactly TXN (1020) bytes and
 *  describes itself, so the two sides can never get out of step:
 *    [0..3] "ACAM" (JPEG) / "AGRY" (80x60 gray) / "ATST" (link test)  [4..7] frame_no
 *    [8..11] frame length  [12..15] offset of this piece  [16..19] sum32 of frame
 *    [20..1020) up to 1000 payload bytes (zero padded)
 *
 *  Speed: core 0 captures + converts + JPEG-encodes the next frame while
 *  core 1 (loop) is sending the previous one over SPI.
 *
 *  Arduino IDE: board "AI Thinker ESP32-CAM", PSRAM enabled.
 * ========================================================================== */
#include "esp_camera.h"
#include "driver/spi_slave.h"
#include "driver/gpio.h"
#include "esp_heap_caps.h"
#include "img_converters.h"

// ---- tuning ----
#define FRAME_SIZE        FRAMESIZE_QVGA   // 320x240 (FRAMESIZE_HQVGA 240x176 / QQVGA 160x120 = faster)
#define SOFT_JPEG_QUALITY 50               // 1..100: lower = smaller frames = higher fps
#define HW_JPEG_QUALITY   12               // 10..63 for sensors with a JPEG encoder (lower = better, bigger)
                                           // 12 ~ 8-10 KB/frame; 20 = old setting (smaller, blockier)
#define LANE_FEATURE      1                // 0 = plain luma, 1 = luma + yellow boost
#define YEL_GAIN          2                // yellow boost gain
#define YEL_T             24               // yellowness below this is ignored (grey road, white balance tint)
#define TEST_EVERY        30               // every 30th frame is a link-test pattern
#define TEST_LEN          3000
#define TW                80               // grayscale thumbnail for ALPANA's lane detection
#define TH                60

// ---- SPI link ----
#define PIN_SCLK  14
#define PIN_MISO  15
#define PIN_CS     2
#define PIN_HS    12
#define SPI_HOST_ID SPI2_HOST
#define TXN       1020          // bytes per transaction (multiple of 20 and 4), must match alpana_cam.c
#define PAY       (TXN - 20)    // payload bytes per transaction

// ---- camera: AI-Thinker pin map ----
#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27
#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22

enum Mode { HW_JPEG, NATIVE_GRAY, RGB565_TO_GRAY };
static Mode mode;
static uint8_t *txbuf;           // DMA-capable buffer (PSRAM is not DMA-able for SPI)
static uint8_t *gray = NULL;     // 8-bit grayscale scratch (RGB565 path)

struct Frame { uint8_t *jpg; size_t len; camera_fb_t *fb; uint16_t w, h; uint32_t enc_ms; uint32_t token; bool has_thumb; };
static uint8_t thumb[2][TW * TH];  // one per frame in flight (indexed by token)

// lane feature of one pixel (8-bit R, G, B): bright for white AND yellow paint
static inline uint8_t lane_feature(uint32_t r, uint32_t g, uint32_t b) {
  int y = (int)((r * 77 + g * 150 + b * 29) >> 8);
#if LANE_FEATURE
  int mn = r < g ? r : g;
  int yel = mn - (int)b - 2 * (((int)r - (int)g - 20) > 0 ? ((int)r - (int)g - 20) : 0);
  if (yel > YEL_T) y += YEL_GAIN * (yel - YEL_T);
  if (y > 255) y = 255;
#endif
  return (uint8_t)y;
}
static inline uint8_t rgb565_feature(const uint8_t *p) {     // big-endian RGB565
  uint16_t v = (p[0] << 8) | p[1];
  return lane_feature((v >> 8) & 0xF8, (v >> 3) & 0xFC, (v << 3) & 0xF8);
}

// w x h 8-bit image -> TW x TH, each cell = (max + mean) / 2 : a thin bright
// line keeps most of its contrast instead of being averaged away
static void make_thumb(const uint8_t *g, int w, int h, uint8_t *out) {
  for (int ty = 0; ty < TH; ty++) {
    int y0 = ty * h / TH, y1 = (ty + 1) * h / TH;
    for (int tx = 0; tx < TW; tx++) {
      int x0 = tx * w / TW, x1 = (tx + 1) * w / TW;
      uint32_t acc = 0, mx = 0;
      for (int y = y0; y < y1; y++) {
        const uint8_t *p = g + y * w;
        for (int x = x0; x < x1; x++) { uint32_t v = p[x]; acc += v; if (v > mx) mx = v; }
      }
      out[ty * TW + tx] = (mx + acc / ((y1 - y0) * (x1 - x0))) >> 1;
    }
  }
}
static QueueHandle_t ready_q;    // encoded frames waiting to be sent
static QueueHandle_t free_q;     // tokens: limits frames in flight to 2

static void IRAM_ATTR on_setup(spi_slave_transaction_t *t) { gpio_set_level((gpio_num_t)PIN_HS, 1); }
static void IRAM_ATTR on_done (spi_slave_transaction_t *t) { gpio_set_level((gpio_num_t)PIN_HS, 0); }

static void put32(uint8_t *p, uint32_t v) { p[0] = v; p[1] = v >> 8; p[2] = v >> 16; p[3] = v >> 24; }

// one fixed-size, self-describing transaction; blocks until ALPANA clocked it out
static void send_piece(const char *magic, uint32_t fno, const uint8_t *data, uint32_t len,
                       uint32_t off, uint32_t sum) {
  uint32_t n = len - off; if (n > PAY) n = PAY;
  memcpy(txbuf, magic, 4);
  put32(txbuf + 4, fno); put32(txbuf + 8, len); put32(txbuf + 12, off); put32(txbuf + 16, sum);
  memcpy(txbuf + 20, data + off, n);
  if (n < PAY) memset(txbuf + 20 + n, 0, PAY - n);
  spi_slave_transaction_t t = {};
  t.length    = TXN * 8;
  t.tx_buffer = txbuf;
  t.rx_buffer = NULL;
  spi_slave_transmit(SPI_HOST_ID, &t, portMAX_DELAY);
}
static void send_frame(const char *magic, uint32_t fno, const uint8_t *data, uint32_t len) {
  uint32_t sum = 0;
  for (uint32_t i = 0; i < len; i++) sum += data[i];
  for (uint32_t off = 0; off < len; off += PAY) send_piece(magic, fno, data, len, off, sum);
}

static bool cam_init(pixformat_t fmt) {
  camera_config_t c = {};
  c.ledc_channel = LEDC_CHANNEL_0;  c.ledc_timer = LEDC_TIMER_0;
  c.pin_d0 = Y2_GPIO_NUM; c.pin_d1 = Y3_GPIO_NUM; c.pin_d2 = Y4_GPIO_NUM; c.pin_d3 = Y5_GPIO_NUM;
  c.pin_d4 = Y6_GPIO_NUM; c.pin_d5 = Y7_GPIO_NUM; c.pin_d6 = Y8_GPIO_NUM; c.pin_d7 = Y9_GPIO_NUM;
  c.pin_xclk = XCLK_GPIO_NUM; c.pin_pclk = PCLK_GPIO_NUM; c.pin_vsync = VSYNC_GPIO_NUM; c.pin_href = HREF_GPIO_NUM;
  c.pin_sccb_sda = SIOD_GPIO_NUM; c.pin_sccb_scl = SIOC_GPIO_NUM;
  c.pin_pwdn = PWDN_GPIO_NUM; c.pin_reset = RESET_GPIO_NUM;
  c.xclk_freq_hz = 20000000;
  c.pixel_format = fmt;
  c.frame_size   = psramFound() ? FRAME_SIZE : FRAMESIZE_QQVGA;
  c.jpeg_quality = HW_JPEG_QUALITY;
  c.fb_count     = psramFound() ? 2 : 1;
  c.fb_location  = psramFound() ? CAMERA_FB_IN_PSRAM : CAMERA_FB_IN_DRAM;
  c.grab_mode    = CAMERA_GRAB_LATEST;
  if (esp_camera_init(&c) == ESP_OK) return true;
  esp_camera_deinit();
  delay(100);
  return false;
}

// ---- core 0: capture -> grayscale -> JPEG, hand over to the sender ----
static void producer(void *) {
  uint32_t token;
  for (;;) {
    xQueueReceive(free_q, &token, portMAX_DELAY);
    camera_fb_t *fb = esp_camera_fb_get();
    if (!fb) { xQueueSend(free_q, &token, 0); vTaskDelay(10); continue; }
    uint32_t t0 = millis();
    Frame f = { fb->buf, fb->len, fb, (uint16_t)fb->width, (uint16_t)fb->height, 0, token, false };
    if (mode == HW_JPEG) {                          // decode a 1/2 (or 1/4) scale colour copy for the thumbnail
      jpg_scale_t sc = f.w >= 4 * 2 * TW ? JPG_SCALE_4X : JPG_SCALE_2X;   // keep >= 160 px wide
      int div = sc == JPG_SCALE_4X ? 4 : 2;
      size_t sw = f.w / div, sh = f.h / div;
      static uint8_t *rgb = NULL, *feat = NULL;
      if (!rgb)  rgb  = (uint8_t *)(psramFound() ? ps_malloc(sw * sh * 2) : malloc(sw * sh * 2));
      if (!feat) feat = (uint8_t *)malloc(sw * sh);
      if (rgb && feat && sw >= TW && sh >= TH && jpg2rgb565(fb->buf, fb->len, rgb, sc)) {
        for (size_t i = 0; i < sw * sh; i++) feat[i] = rgb565_feature(rgb + 2 * i);
        make_thumb(feat, sw, sh, thumb[token]);
        f.has_thumb = true;
      }
    } else {
      size_t npx = (size_t)f.w * f.h;
      const uint8_t *src = fb->buf;
      if (mode == RGB565_TO_GRAY) {
        if (!gray) gray = (uint8_t *)(psramFound() ? ps_malloc(npx) : malloc(npx));
        for (size_t i = 0; i < npx; i++) {        // RGB565 big-endian -> lane feature (for ALPANA)
          gray[i] = rgb565_feature(src + 2 * i);
        }
        make_thumb(gray, f.w, f.h, thumb[token]);
        for (size_t i = 0; i < npx; i++) {        // ... and luma for the video
          uint16_t v = (src[2 * i] << 8) | src[2 * i + 1];
          uint32_t r = (v >> 8) & 0xF8, g = (v >> 3) & 0xFC, b = (v << 3) & 0xF8;
          gray[i] = (uint8_t)((r * 77 + g * 150 + b * 29) >> 8);
        }
        src = gray;
      } else {
        make_thumb(src, f.w, f.h, thumb[token]);     // sensor gray: no colour, plain luma
      }
      f.has_thumb = true;
      uint8_t *jpg = NULL; size_t len = 0;
      bool ok = fmt2jpg((uint8_t *)src, npx, f.w, f.h, PIXFORMAT_GRAYSCALE, SOFT_JPEG_QUALITY, &jpg, &len);
      esp_camera_fb_return(fb);
      if (!ok || !jpg) { xQueueSend(free_q, &token, 0); continue; }
      f.jpg = jpg; f.len = len; f.fb = NULL;
    }
    f.enc_ms = millis() - t0;
    xQueueSend(ready_q, &f, portMAX_DELAY);
  }
}

void setup() {
  Serial.begin(115200);
  Serial.println("\nESP32-CAM -> ALPANA_SOC SPI slave v4 (colour JPEG + 80x60 lane-feature image, yellow boost)");
  Serial.printf("PSRAM: %s\n", psramFound() ? "yes" : "NO");

  if (cam_init(PIXFORMAT_JPEG))            mode = HW_JPEG;
  else if (cam_init(PIXFORMAT_GRAYSCALE))  mode = NATIVE_GRAY;
  else if (cam_init(PIXFORMAT_RGB565))     mode = RGB565_TO_GRAY;
  else { Serial.println("camera init FAILED"); while (1) delay(1000); }
  sensor_t *s = esp_camera_sensor_get();
  if (s) {                                   // colour kept (no grayscale effect): needed to see yellow
    s->set_special_effect(s, 0);
    s->set_whitebal(s, 1);   s->set_awb_gain(s, 1);  s->set_wb_mode(s, 0);
    s->set_exposure_ctrl(s, 1); s->set_aec2(s, 1);   s->set_ae_level(s, 0);
    s->set_gain_ctrl(s, 1);  s->set_gainceiling(s, GAINCEILING_8X);
    s->set_bpc(s, 1); s->set_wpc(s, 1); s->set_raw_gma(s, 1); s->set_lenc(s, 1); s->set_dcw(s, 1);
    s->set_saturation(s, 2);                 // stronger colour -> yellow stands out
    s->set_contrast(s, 1);
    if (s->set_sharpness) s->set_sharpness(s, 1);    // not every sensor supports it
    if (mode == HW_JPEG) s->set_quality(s, HW_JPEG_QUALITY);
  }
  Serial.printf("camera sensor PID 0x%04x, mode %s\n", s ? s->id.PID : 0,
                mode == HW_JPEG ? "hardware JPEG" : mode == NATIVE_GRAY ? "sensor gray + soft JPEG"
                                                                       : "RGB565->gray + soft JPEG");

  // ---- SPI slave ----
  gpio_reset_pin((gpio_num_t)PIN_HS);
  gpio_set_direction((gpio_num_t)PIN_HS, GPIO_MODE_OUTPUT);
  gpio_set_level((gpio_num_t)PIN_HS, 0);
  spi_bus_config_t bus = {};
  bus.mosi_io_num = -1;  bus.miso_io_num = PIN_MISO;  bus.sclk_io_num = PIN_SCLK;
  bus.quadwp_io_num = -1; bus.quadhd_io_num = -1;
  bus.max_transfer_sz = TXN;
  spi_slave_interface_config_t sl = {};
  sl.mode = 3;  sl.spics_io_num = PIN_CS;  sl.queue_size = 1;
  sl.post_setup_cb = on_setup;  sl.post_trans_cb = on_done;
  gpio_set_pull_mode((gpio_num_t)PIN_CS, GPIO_PULLUP_ONLY);
  gpio_set_pull_mode((gpio_num_t)PIN_SCLK, GPIO_PULLUP_ONLY);
  if (spi_slave_initialize(SPI_HOST_ID, &bus, &sl, SPI_DMA_CH_AUTO) != ESP_OK) {
    Serial.println("spi_slave_initialize FAILED"); while (1) delay(1000);
  }
  txbuf = (uint8_t *)heap_caps_malloc(TXN, MALLOC_CAP_DMA);

  // ---- pipeline: 2 frames in flight ----
  ready_q = xQueueCreate(2, sizeof(Frame));
  free_q  = xQueueCreate(2, sizeof(uint32_t));
  for (uint32_t i = 0; i < 2; i++) xQueueSend(free_q, &i, 0);
  xTaskCreatePinnedToCore(producer, "cam", 8192, NULL, 5, NULL, 0);
  Serial.println("ready, waiting for ALPANA to read");
}

// ---- core 1: send frames over SPI ----
void loop() {
  static uint32_t frame_no = 0, sent = 0, t_win = millis(), bytes = 0;
  if (frame_no % TEST_EVERY == TEST_EVERY - 1) {
    static uint8_t pat[TEST_LEN];
    for (int k = 0; k < TEST_LEN; k++) pat[k] = (uint8_t)(k * 7 + frame_no);
    send_frame("ATST", frame_no, pat, TEST_LEN);
    frame_no++;
    return;
  }
  Frame f;
  xQueueReceive(ready_q, &f, portMAX_DELAY);
  if (f.has_thumb) send_frame("AGRY", frame_no, thumb[f.token], TW * TH);   // raw pixels for ALPANA
  send_frame("ACAM", frame_no, f.jpg, f.len);                                // JPEG for the screen
  frame_no++; sent++; bytes += f.len;
  if (f.fb) esp_camera_fb_return(f.fb); else free(f.jpg);
  xQueueSend(free_q, &f.token, portMAX_DELAY);

  uint32_t dt = millis() - t_win;
  if (dt >= 2000) {
    Serial.printf("%ux%u  %.1f fps  avg %u B/frame  encode %u ms\n",
                  f.w, f.h, sent * 1000.0f / dt, (unsigned)(bytes / sent), (unsigned)f.enc_ms);
    sent = 0; bytes = 0; t_win = millis();
  }
}
