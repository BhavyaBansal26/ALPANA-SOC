# esp32cam_lane (v2): lane detection on ALPANA_SOC

ESP32-CAM → SPI1 → **ALPANA** (lane detection in C with the FPU) → steering servo on JA7, plus video with the lane overlay on the PC.

```
ESP32-CAM (colour JPEG 320x240  +  80x60 "lane feature" image) ==SPI mode 3, 5 MHz==> ALPANA SPI1
  ALPANA: lane.c v2 on the 80x60 image -> steering -30..+30 deg -> servo PWM1/JA7, LEDs
  -> UART 2.5 Mbaud -> host/viewer.py: video + lanes + steering + the 80x60 image ALPANA saw
```

## What changed in v2, and why

| Problem | Cause | Fix |
|---|---|---|
| Yellow lines often missed | The camera ran in grayscale. Yellow paint on grey concrete has almost the **same brightness** (≈176 vs ≈161 grey levels), so in B&W it nearly disappears | The sketch now keeps **colour**. The 80x60 image for ALPANA is a *lane feature*: `luma + 2·max(0, yellowness − 24)`, with `yellowness = min(R,G) − B − 2·max(0, R−G−20)`. Yellow becomes as bright as white paint. Grey road, wood and orange stay unchanged |
| Thin and faded lines lost | 4×4 box average when making the thumbnail | 160x120 decode, then each 80x60 cell = (max + mean)/2 |
| Misses with glare or shadow | One global threshold = half of the brightest pixel. A glare spot raised it above the paint | Per row: 33 px **local background**, threshold = 2.5 × road noise (min 12 grey levels), ridge test (paint brighter than the road on both sides) |
| Dashed lines and dropouts | No memory between frames | Line tracking: search near last frame's line, light smoothing, a lost line is **held** for 5 frames |
| Low video quality | JPEG quality 20, and the ALPANA forwarding loop at ~55 cycles per instruction from DDR2 | Hot loops (UART send, SPI receive, row scan) run from **on-chip boot RAM** (`ALPANA_FAST`). JPEG quality 12 |

### Offline test
Synthetic colour roads, 12 frames per case, through the same pipeline. `host/lane_test_gen.py` checks v2 only; the BEFORE column was measured with v1 during development.

| Case | BEFORE (B&W + lane v1)<br>rows found / direction | AFTER (feature + lane v2)<br>rows found / direction |
|---|---|---|
| white on asphalt | 80 % / 100 % | 100 % / 100 % |
| yellow L + white R, asphalt | 60 % / 100 % | 100 % / 100 % |
| yellow on concrete | **0 % / 0 %** | 100 % / 100 % |
| faded yellow on concrete | **0 % / 0 %** | 100 % / 100 % |
| yellow, concrete, curve right | 0 % / 0 % | 100 % / 100 % |
| yellow + glare spot | 0 % / 0 % (18 false) | 100 % / 100 % |
| yellow + tree shadow | 0 % / 0 % | 97 % / 100 % |
| dashed yellow, both sides | 0 % / 0 % | 78 % / 100 % |
| dark tape on light floor | 85 % / 100 % | 100 % / 100 % |
| no lines (blotchy concrete) | LOST ✓ | LOST ✓, 0 false lines |

![before / after](host/before_after.png)

- Run `cd host && python3 lane_test_gen.py`. It writes `lane_test_results.png` and compares the colour feature with plain luma. Plain luma with the v2 algorithm still gets 82 % of directions right; the colour feature gets 100 %.
- Running `lane.c` on the ALPANA core RTL (Verilator) gives **bit-identical** results to the PC build. It takes about **34 k instructions per frame**.

## Build and run
```bash
cd ~/alpana-sdk && source alpana_tools/env.sh
make software PROGRAM=esp32cam_lane
# terminal 1: alpana-ocd
# terminal 2: cd software/examples/alpana_applns/esp32cam_lane/host && python3 viewer.py /dev/ttyUSB1
# terminal 3: alpana-run software/examples/alpana_applns/esp32cam_lane/output/esp32cam_lane.shakti
# then press RST on the ESP32-CAM
```
**Re-flash the ESP32** with `esp32/esp32cam_spi_slave/esp32cam_spi_slave.ino`. Its serial banner must say **v4**. With the old v3 sketch you still get B&W frames, and yellow will still be weak.

### Viewer
- `python3 viewer.py /dev/ttyUSB1 [--gray] [--scale 2] [--save DIR]`
- Keys: `t` toggles the 80x60 inset, `g` toggles gray video, `q` quits.
- The **top-right inset** is the exact 80x60 image ALPANA analysed, sent every 10th frame. Yellow lines must appear **white** in it. If they don't, the sketch is not v4, or `LANE_FEATURE` is 0.
- Line colours: red = measured this frame, orange = held from earlier frames.
- Dots: yellow = left detections, cyan = right detections.

### Tuning
| Where | Option | Default | Notes |
|---|---|---|---|
| sketch | `HW_JPEG_QUALITY` | 12 | 10 = best quality (largest frames). 20 = old setting. Lower fps → raise it |
| sketch | `LANE_FEATURE` | 1 | 0 = plain luma (old behaviour) |
| sketch | `YEL_GAIN`, `YEL_T` | 2, 24 | raise `YEL_T` if a beige/yellowish floor lights up in the inset |
| app | `CAM_BAUD` | 2500000 | 1000000 if your USB-UART drops bytes (`viewer.py --baud 1000000`) |
| app | `THUMB_EVERY` | 10 | how often the 80x60 inset is sent (0 = never) |
| app | `SERVO_INVERT` | 0 | reverse steering |
| lane.c | `MIN_CONTRAST`, `NOISE_K_*` | 12, 2.5× | lower = more sensitive, more false runs |
| lane.c | `HOLD_FRAMES` | 5 | how long a lost line is kept |
| SDK | `ALPANA_FASTRAM` (bsp/include/alpana.h) | 1 | 0 = run everything from DDR2 (if boot-RAM code misbehaves) |

The status line every 3 s prints `fps` and `B/frame`. The `# lane:` line prints `thr` and `noise`. In it, `L`/`R` means measured and `l`/`r` means held.

## Protocol (UART to PC)
- `A5 5A 'J' 'P'` `len fno sum` + JPEG
- `A5 5A 'L' 'N'` `56+2·ROWS`, then the fields: fno, steer, dir, flags, la, lb, ra, rb, cx_top, cx_bot, servo_us, lane_cycles, thr, noise, `lx[12]`, `rx[12]`
  - flags: bit0 L, bit1 R, bit2 dark tape, bit3 L held, bit4 R held
- `A5 5A 'G' 'Y'` `4804` fno + 80x60 pixels
- The viewer also understands the old v1 packets (len 68).
