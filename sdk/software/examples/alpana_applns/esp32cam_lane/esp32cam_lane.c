/***************************************************************************
 * ALPANA SDK example : esp32cam_lane  (v2)
 * ESP32-CAM -> SPI1 -> ALPANA: lane detection (lane.c, FPU) -> steering
 * servo on PWM1 (JA7) + JPEG video and lane overlay to the PC (UART).
 *
 *  Wiring (Nexys 4 DDR <-> AI-Thinker ESP32-CAM):
 *    JB3  SPI1 SCLK  -> GPIO14        JB2 SPI1 MISO <- GPIO15
 *    JD2  CS (GPIO25)-> GPIO2         JD1 HS (GPIO24) <- GPIO12
 *    GND  -- GND                      JA7 PWM1 -> servo signal
 *  ESP32 sketch: ./esp32/esp32cam_spi_slave/ (v4)   PC viewer: ./host/viewer.py
 *
 *  Link protocol v2: every ESP32 SPI transaction is exactly TXN (1020) bytes:
 *    [0..3] "ACAM" JPEG | "AGRY" 80x60 lane-feature image | "ATST" link test
 *    [4..7] frame_no  [8..11] frame length  [12..15] offset  [16..19] sum32
 *  UART to PC (CAM_BAUD):
 *    A5 5A 'J' 'P' len fno sum jpeg
 *    A5 5A 'L' 'N' len(56+2*ROWS) lane result (see send_lane)
 *    A5 5A 'G' 'Y' 4804 fno pixels      every THUMB_EVERY lane frames: what ALPANA sees
 *
 *  v2: hot loops (SPI receive, UART send, lane detection) run from on-chip
 *  boot RAM (ALPANA_FAST): more bytes per second -> better JPEG quality
 *  at the same frame rate.
 * Build: make software PROGRAM=esp32cam_lane
 ***************************************************************************/
#include <stdint.h>
#include "uart.h"
#include "utils.h"
#include "alpana.h"
#include "alpana_spi1.h"
#include "alpana_servo.h"
#include "lane.h"
#undef putchar                          /* use the SDK UART putchar, not the libc macro */
int putchar(int ch);

#ifndef CAM_BAUD
#define CAM_BAUD      2500000           /* use 1000000 if your USB-UART drops bytes */
#endif
#ifndef SPI_BR
#define SPI_BR        7                 /* SCLK = 80 MHz / (2*(BR+1)) = 5 MHz */
#endif
#ifndef SERVO_INVERT
#define SERVO_INVERT  0
#endif
#ifndef THUMB_EVERY
#define THUMB_EVERY   10                /* send the 80x60 lane image every Nth frame (0 = never) */
#endif
#define SERVO_CH      1                 /* PWM1 -> JA7 */
#define HS_PIN        JD1               /* ESP32 "transaction ready" */
#define CS_PIN        JD2               /* ESP32 chip select          */
#define TXN           1020
#define PAY           (TXN - 20)
#define MAX_FRAME     (60 * 1024)
#define MAGIC_CAM     0x4143414Du       /* "ACAM" as a big-endian word */
#define MAGIC_TST     0x41545354u       /* "ATST" */
#define MAGIC_GRY     0x41475259u       /* "AGRY" */

static uint32_t frame[(MAX_FRAME + TXN) / 4];   /* raw big-endian words */
static uint32_t scratch[TXN / 4];
static uint32_t ph[5];                          /* piece header */
static uint8_t  img[LH][LW] __attribute__((aligned(4)));
static lane_result_t lane;
static uint32_t lane_fno, lane_valid, servo_us = 1500, lane_cyc;
static uint32_t n_spi_to, n_hs_stuck, n_nohs;
static const char *dir_name[] = { "STRAIGHT", "LEFT", "RIGHT", "LOST" };

static inline uint8_t fbyte(uint32_t k) { return (uint8_t)(frame[k >> 2] >> (24 - 8 * (k & 3))); }
static void put_u32(uint32_t v) { for (int i = 0; i < 4; i++) alpana_uart_putb_fast((uint8_t)(v >> (8 * i))); }
static uint32_t f2u(float f) { union { float f; uint32_t u; } c; c.f = f; return c.u; }
static void print_hex8(uint32_t v) { const char *h = "0123456789abcdef"; putchar(h[(v >> 4) & 15]); putchar(h[v & 15]); }

/* ---------------------------------------------------- hot loops (boot RAM) */
/* frame[] (big-endian words) -> UART, returns the byte sum */
ALPANA_FAST static uint32_t send_frame_bytes(uint32_t len)
{
	uint32_t s = 0, i = 0;
	const uint32_t *p = frame;
	for (; i + 4 <= len; i += 4) {
		uint32_t v = *p++;
		uint32_t b0 = v >> 24, b1 = (v >> 16) & 0xFF, b2 = (v >> 8) & 0xFF, b3 = v & 0xFF;
		alpana_uart_putb_fast(b0); alpana_uart_putb_fast(b1);
		alpana_uart_putb_fast(b2); alpana_uart_putb_fast(b3);
		s += b0 + b1 + b2 + b3;
	}
	if (i < len) {
		uint32_t v = *p;
		for (int sh = 24; i < len; i++, sh -= 8) { uint8_t b = v >> sh; alpana_uart_putb_fast(b); s += b; }
	}
	return s;
}

ALPANA_FAST static void uart_write(const uint8_t *p, uint32_t n)
{
	while (n--) alpana_uart_putb_fast(*p++);
}

ALPANA_FAST static void unpack_thumb(void)          /* big-endian words -> img[][] */
{
	uint32_t *iw = (uint32_t *)img;
	for (int k = 0; k < LW * LH / 4; k++) {
		uint32_t w = frame[k];
		iw[k] = (w >> 24) | ((w >> 8) & 0xFF00u) | ((w << 8) & 0xFF0000u) | (w << 24);
	}
}

/* ------------------------------------------------------------- ESP32 link */
static int wait_hs(int level, uint32_t ms)
{
	uint64_t end = alpana_cycles() + (uint64_t)ms * (CLOCK_FREQUENCY / 1000);
	while (alpana_gpio_get(HS_PIN) != level) if (alpana_cycles() > end) return 0;
	return 1;
}

/* one ESP32 transaction: 20-byte piece header, then the payload straight to its place */
static int esp_piece(void)
{
	if (!wait_hs(1, 2000)) { n_nohs++; return 0; }
	alpana_gpio_clear(CS_PIN);
	int ok = alpana_spi1_read_be_words(ph, 20) == 0;
	int magic = ph[0] == MAGIC_CAM || ph[0] == MAGIC_TST || ph[0] == MAGIC_GRY;
	uint32_t len = alpana_bswap32(ph[2]), off = alpana_bswap32(ph[3]);
	uint32_t *dst = (magic && len <= MAX_FRAME && off < len && (off % PAY) == 0) ? frame + off / 4 : scratch;
	if (ok) ok = alpana_spi1_read_be_words(dst, PAY) == 0;
	alpana_gpio_set(CS_PIN);
	if (!ok) n_spi_to++;
	if (!wait_hs(0, 20)) n_hs_stuck++;
	if (!ok) return -1;
	return magic ? 1 : -2;
}

/* ------------------------------------------------------------- to the PC */
static void hdr(char a, char b, uint32_t len)
{
	alpana_uart_putb_fast(0xA5); alpana_uart_putb_fast(0x5A); alpana_uart_putb_fast(a); alpana_uart_putb_fast(b);
	put_u32(len);
}

static void send_lane(void)
{
	hdr('L', 'N', 56 + 2 * LANE_ROWS);
	uint32_t flags = (lane.left_ok != 0) | ((lane.right_ok != 0) << 1) | (lane.dark_lines << 2)
	               | ((lane.left_ok == 2) << 3) | ((lane.right_ok == 2) << 4);
	put_u32(lane_fno); put_u32((uint32_t)lane.steer_deg); put_u32(lane.dir); put_u32(flags);
	put_u32(f2u(lane.la)); put_u32(f2u(lane.lb)); put_u32(f2u(lane.ra)); put_u32(f2u(lane.rb));
	put_u32(f2u(lane.cx_top)); put_u32(f2u(lane.cx_bot));
	put_u32(servo_us); put_u32(lane_cyc); put_u32(lane.thr); put_u32(lane.noise);
	for (int i = 0; i < LANE_ROWS; i++) alpana_uart_putb_fast((uint8_t)lane.lx[i]);
	for (int i = 0; i < LANE_ROWS; i++) alpana_uart_putb_fast((uint8_t)lane.rx[i]);
}

static void send_thumb(void)
{
	hdr('G', 'Y', LW * LH + 4);
	put_u32(lane_fno);
	uart_write(&img[0][0], LW * LH);
}

int main(void)
{
	alpana_banner();
	printf("# esp32cam_lane v2: switching UART to %u baud\n", CAM_BAUD);
	alpana_uart_set_baud(CAM_BAUD);

	alpana_gpio_output(CS_PIN | LED_ALL);
	alpana_gpio_input(HS_PIN);
	alpana_gpio_set(CS_PIN);                        /* CS idle high */
	alpana_spi1_init(SPI_BR);
	alpana_servo_init(SERVO_CH);
	lane_init();
	printf("\n# ALPANA_SOC camera link (SDK v2): ESP32-CAM -> SPI1 (%u kHz, mode 3) -> UART %u baud, servo PWM1/JA7, fast RAM %s\n",
	       CLOCK_FREQUENCY / (2 * (SPI_BR + 1)) / 1000, CAM_BAUD, ALPANA_FASTRAM ? "on" : "off");

	uint32_t good = 0, bad_sum = 0, bad_hdr = 0, dropped = 0;
	uint32_t tst_frames = 0, tst_bytes = 0, tst_err = 0, tst_print = 0;
	uint32_t cur_fno = 0, cur_len = 0, have = 0, cur_type = 0; int cur_valid = 0;
	uint32_t lanes = 0, fps_frames = 0, fps_bytes = 0;
	uint64_t t0 = alpana_cycles(), last_stat = t0;

	for (;;) {
		if (alpana_cycles() - last_stat > 3ull * CLOCK_FREQUENCY) {
			uint64_t now = alpana_cycles();
			uint32_t fps100 = (uint32_t)((uint64_t)fps_frames * CLOCK_FREQUENCY * 100 / (now - t0));
			uint32_t avg = fps_frames ? fps_bytes / fps_frames : 0;
			printf("# status: ok %u | bad csum %u hdr %u dropped %u | spi_to %u hs %u nohs %u HS=%d | test %u frames, %u bad bytes of %u | fps %u.%u%u, %u B/frame\n",
			       good, bad_sum, bad_hdr, dropped, n_spi_to, n_hs_stuck, n_nohs, alpana_gpio_get(HS_PIN),
			       tst_frames, tst_err, tst_bytes, fps100 / 100, (fps100 / 10) % 10, fps100 % 10, avg);
			last_stat = now; t0 = now; fps_frames = 0; fps_bytes = 0;
		}

		int r = esp_piece();
		if (r <= 0) {
			if (r == 0 && n_nohs % 3 == 1) printf("# no handshake from ESP32 (HS=%d) - check JD1/GPIO12, GND, sketch\n", alpana_gpio_get(HS_PIN));
			if (r == -2 && bad_hdr++ < 6) {
				printf("# bad piece header:");
				for (int i = 0; i < 5; i++) for (int j = 24; j >= 0; j -= 8) { putchar(' '); print_hex8(ph[i] >> j); }
				printf("\n");
			}
			cur_valid = 0;
			continue;
		}
		uint32_t fno = alpana_bswap32(ph[1]), len = alpana_bswap32(ph[2]);
		uint32_t off = alpana_bswap32(ph[3]), sum = alpana_bswap32(ph[4]);
		if (len == 0 || len > MAX_FRAME || off >= len) { bad_hdr++; cur_valid = 0; continue; }

		/* assemble: pieces must arrive in order 0, PAY, 2*PAY ... */
		if (off == 0) {
			if (cur_valid && have < cur_len) dropped++;
			cur_valid = 1; cur_fno = fno; cur_len = len; cur_type = ph[0]; have = 0;
		}
		if (!cur_valid || fno != cur_fno || off != have || ph[0] != cur_type) { if (cur_valid) dropped++; cur_valid = 0; continue; }
		have += (len - off < PAY) ? len - off : PAY;
		if (have < cur_len) continue;
		cur_valid = 0;

		/* 80x60 lane-feature image: lane detection + steering servo */
		if (cur_type == MAGIC_GRY) {
			if (len != LW * LH) continue;
			uint64_t c0 = alpana_cycles();
			unpack_thumb();
			lane_detect((const uint8_t (*)[LW])img, &lane);
			int steer = lane.dir == DIR_LOST ? 0 : lane.steer_deg;
			servo_us = alpana_servo_set_angle(SERVO_CH, SERVO_INVERT ? -steer : steer, STEER_MAX_DEG);
			alpana_gpio_clear(LED_ALL);                  /* LEDs show the decision */
			alpana_gpio_set(lane.dir == DIR_LEFT ? LED0 : lane.dir == DIR_RIGHT ? LED3 :
			                lane.dir == DIR_STRAIGHT ? (LED1 | LED2) : 0);
			lane_cyc = (uint32_t)(alpana_cycles() - c0);
			lane_fno = fno; lane_valid = 1; lanes++;
			if (THUMB_EVERY && lanes % THUMB_EVERY == 1) send_thumb();
			if ((lanes & 15) == 1)
				printf("# lane: %s %d deg | servo %u us | lines %s%s%s | thr %d noise %d | %u.%u ms\n", dir_name[lane.dir], lane.steer_deg,
				       servo_us, lane.left_ok ? (lane.left_ok == 2 ? "l" : "L") : "-",
				       lane.right_ok ? (lane.right_ok == 2 ? "r" : "R") : "-",
				       lane.dark_lines ? " (dark tape)" : "", lane.thr, lane.noise,
				       lane_cyc / (CLOCK_FREQUENCY / 1000), (lane_cyc / (CLOCK_FREQUENCY / 10000)) % 10);
			continue;
		}

		/* link test frame: compare every byte with the known pattern */
		if (cur_type == MAGIC_TST) {
			uint32_t e = 0;
			for (uint32_t k = 0; k < len; k++) {
				uint8_t exp = (uint8_t)(k * 7 + fno), got = fbyte(k);
				if (got != exp) {
					if (e == 0 && tst_print < 6) {
						tst_print++;
						printf("# test %u: first bad byte %u: got ", fno, k); print_hex8(got);
						printf(" exp "); print_hex8(exp); printf("\n");
					}
					e++;
				}
			}
			tst_frames++; tst_bytes += len; tst_err += e;
			continue;
		}

		if (fbyte(0) != 0xFF || fbyte(1) != 0xD8) {
			if (bad_sum++ < 6) printf("# frame %u: no JPEG start marker\n", fno);
			continue;
		}

		/* stream to PC; sum32 checked while sending (no extra pass) */
		hdr('J', 'P', len);
		put_u32(fno); put_u32(sum);
		uint32_t s = send_frame_bytes(len);
		if (s != sum && bad_sum++ < 6) printf("# frame %u: checksum %x vs %x\n", fno, s, sum);
		if (lane_valid) send_lane();
		good++; fps_frames++; fps_bytes += len;
	}
	return 0;
}
