# ALPANA SDK: Developer Guide

**ALPANA SDK** is the software development kit for **ALPANA_SOC**:
- the Shakti E-class core, made 4-stage, with a single-precision FPU (RV32IMAFC);
- running at 80 MHz on the Digilent **Nexys 4 DDR** (xc7a100t), with 128 MB of DDR2.

It is a fork of IIT-Madras **shakti-sdk**. All the original boards and examples still build, and `alpana` is the default target.

---

## 1. Contents
1. Contents
2. Quick start
3. ALPANA_SOC hardware view (memory map, pins)
4. Directory structure
5. Build system: how `make software` works
6. Boot and program life-cycle
7. Loading, running and debugging
8. Driver reference: ALPANA drivers
9. Driver reference: stock Shakti drivers on ALPANA
10. Examples
11. Benchmarks
12. Writing your own application
13. Performance guide (no-cache core)
14. Troubleshooting
15. Known limitations
16. Changes compared with shakti-sdk

---

## 2. Quick start
```bash
# one time
cd ~/alpana-sdk && source alpana_tools/env.sh      # sets ALPANASDK + aliases

# build
make software PROGRAM=alpana_hello                  # TARGET=alpana is the default

# run (3 terminals)
alpana-ocd                                          # 1: OpenOCD (JTAG over the Nexys USB)
alpana-uart                                         # 2: UART console, 115200 on /dev/ttyUSB1
alpana-run software/examples/alpana_applns/alpana_hello/output/alpana_hello.shakti   # 3: load + run
```
Before running, two things must already be done:
- `fpga_top.bit` is programmed on the board;
- the Nexys USB is attached to WSL (`usbipd attach --wsl --busid <id>`).

Expected UART output:
```
==============================================
 ALPANA_SOC  (Shakti E-class 4-stage + SP FPU)
 RV32ACFIMSU  @ 80 MHz   DDR2 128 MB @ 0x80000000
==============================================
 100 ms delay = 31250 mtime ticks (expect 31250)
 FPU: 1.5*2.25 + 1.5/2.25 = 4.0416 (expect 4.0416)
 loop: ... CPI ...
 Hello from ALPANA_SOC!
```

---

## 3. ALPANA_SOC hardware view

### 3.1 SoC summary
| Item | Value |
|---|---|
| Core | Shakti E-class, 4-stage pipeline, RV32IMAFC (+ single-precision FPU), machine mode |
| Clock | 80 MHz core (`clk_wiz` clk_out1) |
| Caches | **none** (see §13) |
| Main memory | 128 MB DDR2 (MIG, AXI4 32-bit) at `0x8000_0000` |
| Boot RAM | 8 KB on-chip BRAM at `0x0000_1000` (boot code; writable) |
| Debug | RISC-V debug 0.13 through a BSCANE2 JTAG tunnel (USER4, irlen 6) |

### 3.2 Memory map (`bsp/third_party/alpana/platform.h`)
| Region | Base | Notes |
|---|---|---|
| Boot RAM | `0x0000_1000` – `0x0000_2FFF` | boot code; `0x2C00` is the park location used at program exit |
| UART0 / UART1 / UART2 | `0x0001_1300` / `+0x100` / `+0x200` | UART0 = USB-UART (`/dev/ttyUSB1`); UART1/2 through the pin-mux on JA |
| SPI0 | `0x0002_0000` | on-board Quad-SPI flash (configuration flash) |
| SPI1 | `0x0002_0100` | Pmod JA10/JB1–JB3 (driver: `alpana_spi1.h`) |
| PWM0 … PWM5 | `0x0003_0000` + n·`0x100` | driver: `alpana_servo.h` or the stock `pwm_driver.h` |
| I2C0 | `0x0004_0000` | on-board ADT7420 temperature sensor |
| GPIO | `0x0004_0100` | DIR `+0x00` (1 = output), DATA `+0x08` |
| XADC | `0x0004_1000` | **port not connected in `fpga_top`: do not access** |
| I2C1 | `0x0004_1400` | Pmod JB4 (SCL) / JB10 (SDA) |
| Pin-mux | `0x0004_1510` | 2 bits per I/O cell (§3.3) |
| CLINT | `0x0200_0000` | `mtime` at `+0xBFF8`, ticks every **256** cycles (312.5 kHz) |
| PLIC | `0x0C00_0000` | 28 sources (PWM, GPIO, I2C, UART) |
| DDR2 | `0x8000_0000` – `0x87FF_FFFF` | program, data, stack and heap |

### 3.3 Board I/O and pins
**On-board (dedicated GPIOs)**

| Signal | GPIO | SDK name |
|---|---|---|
| LD0–LD3 | 16–19 | `LED0`…`LED3`, `LED_ALL` |
| SW0, SW1, SW2, SW3, SW4 | 4, 7, 8, 14, 15 | `SW0`…`SW4` |
| BTNC, BTNU, BTNL, BTNR | 20–23 | `BTN_C`, `BTN_U`, `BTN_L`, `BTN_R` |
| BTND, SW15 | PLIC external interrupts 0/1 | – |

**Pmod JD**: 8 plain GPIOs (3.3 V, pull-down)

| JD pin | 1 | 2 | 3 | 4 | 7 | 8 | 9 | 10 |
|---|---|---|---|---|---|---|---|---|
| GPIO | 24 | 25 | 26 | 27 | 28 | 29 | 30 | 31 |
| SDK name | `JD1` | `JD2` | `JD3` | `JD4` | `JD7` | `JD8` | `JD9` | `JD10` |

**Pmod JA / JB**: pin-muxed I/O cells. Set them with `alpana_pinmux_set(PIN_xx, PINMUX_…)`:

| Pmod pin | 0 `PINMUX_GPIO` | 1 `PINMUX_ALT1` | 2 `PINMUX_ALT2` |
|---|---|---|---|
| JA1 | GPIO | UART1 RX | – |
| JA2 | GPIO | UART1 TX | – |
| JA3 | GPIO | UART2 RX | – |
| JA4 | GPIO | UART2 TX | PWM0 |
| JA7 | GPIO | – | PWM1 |
| JA8 | GPIO | – | PWM2 |
| JA9 | GPIO | – | PWM3 |
| JA10 | GPIO | SPI1 NSS | PWM4 |
| JB1 | GPIO | SPI1 MOSI | PWM5 |
| JB2 | GPIO | SPI1 MISO | – |
| JB3 | GPIO | SPI1 SCLK | – |
| JB4 / JB10 | I2C1 SCL / SDA (fixed) | | |

Pmod pin numbering: the top row is 1-2-3-4-GND-3V3 and the bottom row is 7-8-9-10-GND-3V3.

---

## 4. Directory structure
ALPANA-specific items are marked ★. Everything else is stock shakti-sdk.
```
alpana-sdk/
├── README.md / README_ALPANA.md ★     overview + quick reference
├── Makefile                           top level: TARGET ?= alpana ★, software / project / list_* targets
├── setenv.sh, gdb.script              stock helpers
├── alpana_tools/ ★
│   ├── env.sh                         exports ALPANASDK, aliases alpana-ocd / alpana-run / alpana-uart
│   ├── alpana_openocd.cfg             OpenOCD: FT2232H ch.A, BSCAN tunnel, irlen 6
│   └── run.gdb                        connect :3333, halt, load, pc=_start, continue
├── bsp/
│   ├── core/                          start.S ★(FPU on), init.c ★(UART 115200 + park), trap.S, traps.c
│   ├── drivers/
│   │   ├── alpana/ ★                  alpana_board.c, alpana_spi1.c, alpana_servo.c
│   │   └── clint, i2c, plic, pwm, qspi, spi, uart, xadc, ethernet   (stock)
│   ├── include/                       alpana.h ★, alpana_spi1.h ★, alpana_servo.h ★ + stock headers
│   ├── libs/                          printf.c, util.c, log.c, malloc, board LED/switch libs (stock)
│   ├── third_party/
│   │   ├── alpana/ ★                  platform.h, link.ld, ftdi.cfg
│   │   └── parashu, pinaka, vajra, moushik, artix7_35t, artix7_100t   (stock boards)
│   └── utils/uploader/                flash uploader (stock, Arty/spansion oriented)
├── doc/                               Shakti user/boot/PLIC manuals (+ this guide ★)
└── software/
    ├── examples/
    │   ├── Makefile ★                 alpana target, ALPANA drivers in libshakti32.a, auto-discovery of PROGRAM
    │   ├── gen_lib/                   generated: *.o + libshakti32.a (rebuilt on every make)
    │   ├── alpana_applns/ ★           alpana_hello, alpana_blink, servo_sweep, esp32cam_lane
    │   └── uart_ gpio_ i2c_ spi_ pwm_ plic_ clint_ xadc_ malloc_ eth_ applns   (stock examples)
    ├── alpana_bench/ ★                standalone perf / dhrystone / coremark / stress suite
    ├── projects/                      stock Shakti projects (weather station, uart-cam, lora, ...)
    └── benchmarking/paranoia          stock FP test
```
`output/` folders (`<app>.o`, `.shakti` ELF and `.dump` disassembly) and `gen_lib/` are build products. They are safe to delete.

---

## 5. Build system: how `make software` works
```
make software PROGRAM=<name> [TARGET=alpana] [DEBUG=DEBUG]
   │
   ├─ top Makefile ── exports bsp paths (bspboard = bsp/third_party/$(TARGET), bspinc, bspdri, ...)
   │                  └─ cd software/examples && make PROGRAM=.. TARGET=..
   │
   └─ software/examples/Makefile
        1. check    : TARGET=alpana -> XLEN=32, MARCH=rv32imafc[_zicsr_zifencei], MABI=ilp32f, FLAGS=-D ALPANA
        2. library  : compiles start.S, init.c, traps, printf, util, uart, i2c, plic, clint, pwm, qspi, xadc,
                      eth + ALPANA drivers  ->  gen_lib/libshakti32.a
        3. do       : finds the app folder: stock list, else any <group>/<PROGRAM>/  (auto-discovery)
        4. finish   : cd <group>/<PROGRAM> && make <PROGRAM>.riscv
                        -> output/<PROGRAM>.shakti   (ELF, linked with bsp/third_party/alpana/link.ld)
                        -> output/<PROGRAM>.dump     (objdump -D)
```
**Toolchain**
- The build uses `riscv32-unknown-elf-gcc` (a symlink to a riscv64 multilib toolchain also works). It needs the **rv32imafc/ilp32f** multilib.
- With GCC ≥ 12 the Makefile appends `_zicsr_zifencei` to `-march` automatically.
- ALPANA apps compile with `-O2`; stock apps use the toolchain default (`-O0`).

Other useful commands:
```bash
make list_targets                              # alpana artix7_100t artix7_35t moushik parashu pinaka vajra
make software PROGRAM=hello                    # stock examples also build for ALPANA
make software PROGRAM=alpana_hello DEBUG=DEBUG # adds -g
cd software/examples && make clean             # removes all output/ folders
```

---

## 6. Boot and program life-cycle
```
FPGA config ─> boot code in boot RAM (0x1000) ─> waits for the debugger
gdb "load" writes the ELF into DDR2, sets pc=_start, continue
   │
   ├─ bsp/core/start.S   mtvec=trap_entry, interrupts off, FPU on (mstatus.FS, fcsr=0),
   │                     PLIC enables cleared, registers zeroed, gp, .bss/.sbss cleared, sp=top of stack
   ├─ bsp/core/init.c    uart_init(); UART0 -> 115200 (ALPANA); trap table; main()
   └─ main() returns  ─> alpana_park(): interrupts off, write "j ." at 0x2C00, jump there
```
**Why park?** The debugger can't halt the core while it keeps fetching from DDR2. In boot RAM it can, so the next `alpana-run` works without pressing CPU RESET. Programs that loop forever (`while(1)`) don't park. To load the next program after one of those, use the recovery steps in §14.

**Linker layout (`link.ld`):**
- Code and data start at `0x8000_0000`.
- `_end` marks the end of the program.
- The free DDR2 above it is split 50/50: stack (top of the 128 MB, growing down) and heap.

---

## 7. Loading, running and debugging
| Step | Command |
|---|---|
| Attach USB (Windows, admin) | `usbipd attach --wsl --busid <nexys-busid>` → `/dev/ttyUSB0` (JTAG), `/dev/ttyUSB1` (UART) |
| OpenOCD | `alpana-ocd`, i.e. `openocd -c "debug_level 0" -f alpana_tools/alpana_openocd.cfg` |
| UART console | `alpana-uart`, i.e. `picocom -b 115200 --imap lfcrlf /dev/ttyUSB1` |
| Load + run | `alpana-run <file>.shakti`, i.e. `riscv32-unknown-elf-gdb -q -x alpana_tools/run.gdb <file>` |
| Interactive debug | `riscv32-unknown-elf-gdb <file>.shakti` → `target extended-remote :3333`, `monitor halt`, `load`, `break main`, `continue`, `stepi`, `info registers` |
| Raw memory | `telnet localhost 4444` → `mdw 0x80000000 8`, `mww <addr> <val>` |

- **Harmless messages:** `keep_alive() was not invoked ... (17xxxx ms)` comes from the usbipd clock and can be ignored. Add `-c "debug_level 0"` to hide it.
- **Hardware breakpoints:** OpenOCD reports `Found 0 triggers`, which means there are none. Use `stepi`, or software breakpoints on code in DDR2.

---

## 8. Driver reference: ALPANA drivers
They are built into `libshakti32.a` when `TARGET=alpana`. Include the header and call the functions.

### 8.1 `alpana.h`: board, GPIO, pin-mux, time, UART helpers
| Function | Description |
|---|---|
| `void alpana_gpio_output(uint32_t mask)` / `alpana_gpio_input(mask)` | set direction (1 = output) |
| `void alpana_gpio_set(mask)` / `clear(mask)` / `toggle(mask)` | drive outputs (driver keeps a shadow of the data register) |
| `uint32_t alpana_gpio_read(void)` / `int alpana_gpio_get(mask)` | read pin levels |
| `void alpana_pinmux_set(int pin, int function)` | `PIN_JA1 … PIN_JB3`, `PINMUX_GPIO / _ALT1 / _ALT2` (§3.3) |
| `uint64_t alpana_cycles(void)` / `alpana_instret(void)` | 64-bit `mcycle` / `minstret` |
| `void alpana_delay_us(uint32_t)` / `alpana_delay_ms(uint32_t)` | busy-wait on `mcycle` |
| `uint32_t alpana_millis(void)` | ms since reset |
| `uint32_t alpana_banner(void)` | prints the SoC banner, returns `misa` |
| `void alpana_uart_putb(uint8_t)` | raw UART0 byte (binary streams, no CR/LF handling) |
| `alpana_uart_putb_fast(uint8_t)` | same, `static inline`, for `ALPANA_FAST` loops |
| `void alpana_uart_set_baud(uint32_t)` | waits for TX empty (max 100 ms), then sets the UART0 baud rate (e.g. 1000000, 2500000) |
| `ALPANA_UART0_TX` / `ALPANA_UART0_STATUS` | volatile UART0 registers. The stock `uart_struct` fields are **not** volatile, so a polling loop on `uart_instance[0]->status` becomes `j .` at `-O2` |
| `ALPANA_FAST` / `ALPANA_FAST_BSS` | put a function / data in on-chip boot RAM (§8.4) |

```c
#include "alpana.h"
alpana_gpio_output(LED_ALL);
alpana_gpio_input(BTN_C);
while (1) { if (alpana_gpio_get(BTN_C)) alpana_gpio_toggle(LED0); alpana_delay_ms(100); }
```

### 8.2 `alpana_spi1.h`: SPI1 master receive
- **Mode:** mode 3 (CPOL=1, CPHA=1), MSB first.
- **Bursts:** hardware bursts of **20 bytes** (DR1..DR5, big-endian).
- **Clock:** `SCLK = 80 MHz / (2·(br+1))`, so br 7 → 5 MHz and br 3 → 10 MHz.
- **Chip select:** use any GPIO (e.g. `JD2`) and hold it low for the whole transfer, so the slave sees one continuous transaction.

| Function | Description |
|---|---|
| `void alpana_spi1_init(int br)` | pin-mux JA10/JB1–JB3 to SPI1; receive config |
| `int alpana_spi1_read(uint8_t *dst, uint32_t len)` | `len` multiple of 20; bytes in stream order; 0 = OK, <0 = error/timeout |
| `int alpana_spi1_read_be_words(uint32_t *dst, uint32_t len)` | fastest; stores raw big-endian words (`alpana_bswap32()` to convert) |

```c
alpana_gpio_output(JD2); alpana_gpio_set(JD2);        /* CS idle high */
alpana_spi1_init(7);                                   /* 5 MHz */
uint8_t buf[200];
alpana_gpio_clear(JD2); alpana_spi1_read(buf, 200); alpana_gpio_set(JD2);
```
**How it was verified:** in RTL simulation of `mkspi_e32.v`, with 20-byte bursts and the next burst started while the previous one is copied out, there were 0 byte errors. On the board, the ESP32-CAM link moved about 2.5 MB with 0 bad bytes.

### 8.3 `alpana_servo.h`: PWM in microseconds, hobby servos
| Function | Description |
|---|---|
| `void alpana_pwm_init_us(int ch, uint32_t period_us, uint32_t high_us)` | 1 µs ticks; period ≤ 65535 µs; sets the pin-mux for the channel |
| `void alpana_pwm_set_us(int ch, uint32_t high_us)` | change the pulse width |
| `void alpana_pwm_stop(int ch)` | output off |
| `void alpana_servo_init(int ch)` | 50 Hz, 1500 µs (centre) |
| `void alpana_servo_set_us(int ch, uint32_t us)` | clamped to 500–2500 µs |
| `uint32_t alpana_servo_set_angle(int ch, int deg, int range)` | −range…+range → 1000…2000 µs; returns the µs written |

Channel → pin: PWM0 JA4, **PWM1 JA7**, PWM2 JA8, PWM3 JA9, PWM4 JA10, PWM5 JB1.

**Register-width rule (from the RTL):** period, duty and clock need **16-bit** writes, and control needs an **8-bit** write. The driver handles this.

**How it was verified:** in RTL simulation, the servo settings produce exactly a 20.000 ms period with a 1.500 ms pulse.

**Servo wiring:**
- Signal → JA7.
- + → an **external 5 V** supply, never the Nexys 3V3.
- − → that supply's GND **and** the Nexys GND.

### 8.4 `ALPANA_FAST`: hot code in on-chip boot RAM
The core has no cache, so code in DDR2 runs at about 55 cycles per instruction. The boot RAM (0x1000–0x2FFF) is on-chip BRAM on the same bus. The boot code uses only 0x1000–0x1C24 (and its stack only while it runs at reset), so the SDK uses the rest:

| Area | Address | Size | Marker |
|---|---|---|---|
| code | 0x1C80–0x2BFF | 3968 B | `ALPANA_FAST` (also `noinline`) |
| park word | 0x2C00 | 4 B | `alpana_park()` after `main()` |
| data | 0x2C10–0x2FBF | 944 B | `ALPANA_FAST_BSS` (zeroed before `main()`) |

How it works:
- `link.ld` links `.fasttext` at 0x1C80 but stores it in DDR2 (LMA after `.bss`). gdb `load` writes it there.
- `init()` copies it to boot RAM and runs `fence.i` before `main()`. Calls between DDR2 and boot RAM are normal calls.
- The linker stops with an error if an area overflows.
- Put only per-pixel / per-byte loops there; code that runs once per frame gains little.
- `esp32cam_lane` uses about 1.7 KB of the code area: UART send loop, SPI1 receive, lane row scan and line fit.
- Set `ALPANA_FASTRAM` to 0 in `bsp/include/alpana.h` to build everything for DDR2, for example to rule this out while debugging.

```c
#include "alpana.h"
ALPANA_FAST_BSS static int32_t buf[64];
ALPANA_FAST static uint32_t sum_words(const uint32_t *p, int n) { uint32_t s = 0; while (n--) s += *p++; return s; }
```

---

## 9. Driver reference: stock Shakti drivers on ALPANA
| Driver (header) | On ALPANA | Notes |
|---|---|---|
| UART (`uart.h`) | ✅ | `printf` → UART0. UART1/2 need the pin-mux (JA1–JA4). No width/padding in `printf` (`%d %u %x %s %c %f` only) |
| CLINT (`clint_driver.h`) | ✅ | `mtime` = clock/256 → `CLINT_DIVISOR 256` |
| PLIC (`plic_driver.h`) | ✅ | 28 sources as in parashu |
| GPIO (`gpio.h`) | ✅ | the `alpana.h` helpers are simpler |
| PWM (`pwm_driver.h`) | ✅ | stock API; set the pin-mux yourself |
| I2C (`i2c.h`) | ✅ | I2C0 = on-board ADT7420, I2C1 = JB4/JB10 |
| SPI flash (`spi.h`, spansion / w25q32) | ⚠️ | SPI0 is wired to the configuration flash. Writing it can erase your bitstream |
| GPIO soft-SPI / soft-I2C (`gpio_spi.h`, `gpio_i2c.h`) | ✅ | use them for SPI writes |
| XADC (`xadc_driver.h`) | ❌ | XADC port not connected in `fpga_top`; an access will stall the bus |
| Ethernet (`eth_driver.h`) | ❌ | no Ethernet MAC in ALPANA_SOC |
| Flash uploader (`make upload`) | ❌ | made for the Arty spansion flash. Use gdb load |

---

## 10. Examples (`software/examples/alpana_applns/`)
| PROGRAM | Hardware | What it shows |
|---|---|---|
| `alpana_hello` | board only | banner, `misa`, `mcycle` vs CLINT check, FPU arithmetic, loop CPI; returns → park |
| `alpana_blink` | LEDs, buttons, switches | LED chase; BTNC pause, BTNL/BTNR direction, SW0 speed; prints input changes |
| `servo_sweep` | servo on JA7 | ±30° sweep at 50 Hz; LEDs show the direction |
| `esp32cam_lane` | ESP32-CAM + servo | the full pipeline below |

**`esp32cam_lane` pipeline (v2, sketch v4):**
```
OV2640 (colour) -> ESP32-CAM: JPEG 320x240 (quality 12)
                             + 80x60 lane-feature image (luma + yellow boost, (max+mean)/2 pooling)
   ==SPI mode 3, 5 MHz==> ALPANA SPI1
   ALPANA: assemble frames, lane.c v2 on 80x60 (local background threshold, ridge test,
           line tracking + hold, FPU line fit; ~34 k instructions, row scan in boot RAM)
           steering -30..+30 deg -> servo PWM1/JA7, LEDs = LEFT/STRAIGHT/RIGHT
   -> UART 2.5 Mbaud -> host/viewer.py: video + lane lines + steering arrow + 80x60 inset
```
Why v2: in B&W, yellow paint on grey concrete has nearly the same brightness as the road, so v1 missed it. A single bright spot also raised v1's global threshold above the paint. On the synthetic yellow-road set, v1 found **0 %** of the yellow lines; v2 finds 97–100 % (details and before/after picture in the app's `README.md`).

**Wiring:**

| Nexys | ESP32-CAM / servo |
|---|---|
| JB3 SCLK | GPIO14 |
| JB2 MISO | GPIO15 |
| JD2 CS | GPIO2 |
| JD1 HS | GPIO12 |
| GND | GND |
| JA7 | servo signal |

**Files:**
- `esp32/esp32cam_spi_slave/`: Arduino sketch **v4** (board "AI Thinker ESP32-CAM", PSRAM on). Unplug GPIO2 while flashing. Options: `HW_JPEG_QUALITY` (12), `LANE_FEATURE` (1 = yellow boost), `YEL_GAIN`/`YEL_T`.
- `host/viewer.py`: `python3 viewer.py /dev/ttyUSB1 [--gray]`. Use `--baud 1000000` if you build with `-DCAM_BAUD=1000000`. Keys: `t` toggles the 80x60 inset (what ALPANA sees), `g` toggles gray video.
- `host/lane_test_gen.py`: offline test of `lane.c` on synthetic colour roads (`lane_test_results.png`, `before_after.png`).

**Build options** (in the app Makefile `FLAGS`, or by editing the defines):

| Option | Default | Meaning |
|---|---|---|
| `CAM_BAUD` | 2500000 | UART0 baud for the video stream |
| `SPI_BR` | 7 | SPI1 clock = 80 MHz / (2·(BR+1)) |
| `SERVO_INVERT` | 0 | reverse the steering direction |
| `THUMB_EVERY` | 10 | send the 80x60 lane image to the viewer every Nth frame (0 = off) |

**Results measured on the board (v1):** about 13.8 fps of video with 0 SPI byte errors over 2.5 MB of link tests.
v2 sends larger, better JPEGs (quality 12) and moves the byte loops to boot RAM. Check the `# status:` line (`fps`, `B/frame`); if fps is lower than you want, raise `HW_JPEG_QUALITY` in the sketch (e.g. 15).

---

## 11. Benchmarks (`software/alpana_bench/`)
This is a standalone bare-metal suite, independent of `libshakti`. It has its own `crt.S`, `lib.c` and `link.ld`.
```bash
cd software/alpana_bench && make            # build/perf.elf dhry.elf coremark.elf stress.elf
riscv32-unknown-elf-gdb -q -x run.gdb build/coremark.elf
```
| Program | Measures |
|---|---|
| `perf` | instruction latency / throughput, memory bandwidth, trap/interrupt latency, 4 kernels with CPI/MIPS/MFLOPS |
| `dhry` | Dhrystone 2.1 (20000 runs) |
| `coremark` | CoreMark 1.0 (400 iterations, ≥10 s run) |
| `stress` | random ALU/FPU kernels vs host golden, 1 MB DDR2 March test, AMO/LR/SC, CSRs (incl. `misa` extension check), interrupt storm, 10-round soak |

Measured on the board (code in DDR2, no cache) and in simulation (fast memory):

| Metric | Board | Simulation |
|---|---|---|
| Dhrystone DMIPS/MHz | 0.015 | 0.111 |
| CoreMark/MHz | 0.053 | 0.400 |
| Typical CPI | 55–70 | 8–9 |
| Stress | all pass | all pass |

---

## 12. Writing your own application
```bash
cd ~/alpana-sdk/software/examples/alpana_applns
mkdir myapp && cp alpana_hello/Makefile myapp/
cat > myapp/myapp.c <<'EOF'
#include "uart.h"
#include "utils.h"
#include "alpana.h"
int main(void) {
    alpana_banner();
    alpana_gpio_output(LED_ALL);
    for (int i = 0; i < 10; i++) { alpana_gpio_toggle(LED0); alpana_delay_ms(200); }
    printf("done\n");
    return 0;                       /* parks the core -> next program loads without reset */
}
EOF
cd ~/alpana-sdk && make software PROGRAM=myapp
alpana-run software/examples/alpana_applns/myapp/output/myapp.shakti
```
**App Makefile settings:**
- `EXTRA_SRC ?= ./a.c ./b.c` adds more source files.
- `OPT ?= -O2` sets the optimisation level.
- `FLAGS += -DMYOPT=1` adds defines.

**Folder rule:** `<group>/<name>/<name>.c` builds with `PROGRAM=<name>`, without editing any list.

**Notes:**
- `printf` has no width/padding support. Print hex digits yourself if you need fixed width.
- Use `alpana_uart_putb` for binary data.
- If you call `putchar` directly, add `#undef putchar` after the includes, so the SDK UART function is used and not the libc macro.
- Floating point is native single precision (`float`). `double` is emulated in software and slow.

---

## 13. Performance guide (no-cache core)
E-class has no instruction or data cache. With DDR2 as main memory, **every instruction fetch costs about 48 core cycles**. For comparison:
- simple instruction: 48 cycles;
- load/store: about 96 cycles;
- taken branch: about 144 cycles.

Practical rules:
1. Build with `-O2` (the default for ALPANA apps). Fewer instructions is the single biggest win.
2. Use **word loads** over byte loads. One `lw` plus shifts beats four `lbu` from DDR2.
3. Keep inner loops short and avoid function calls inside them. Use `static inline` / `always_inline`.
4. Fuse passes: compute checksums or statistics while copying data, not in a second loop.
5. Overlap I/O with compute where the peripheral allows it. `alpana_spi1` starts the next burst before copying the current one.
6. Budget: about 1.5 MIPS of effective throughput. That is enough for control loops, 80×60 vision (lane, motion, blob tracking) and protocol handling. It is not enough for full-resolution image processing or neural networks.
7. Put per-pixel / per-byte loops in **on-chip boot RAM** with `ALPANA_FAST` (§8.4). This needs no hardware change.
8. The full fix is hardware: a small I-cache. That should bring CPI back to about 9, the simulation figure.

---

## 14. Troubleshooting
| Symptom | Cause | Fix |
|---|---|---|
| Nothing on the UART | wrong port/baud, or USB not attached | `ls /dev/ttyUSB*`, use `ttyUSB1` at 115200; redo `usbipd attach` |
| Garbage on the UART | baud mismatch | ALPANA SDK apps start at 115200. `esp32cam_lane` switches to 2.5 Mbaud (use the viewer, not picocom) |
| `unable to halt hart 0` / `didn't halt coming out of reset` | core is running a program from DDR2 | 1) Ctrl-C/quit gdb, 2) press CPU RESET or reprogram the `.bit`, 3) restart OpenOCD, 4) load again. SDK programs that return from `main()` avoid this |
| `keep_alive() was not invoked` | usbipd timing | harmless; `-c "debug_level 0"` |
| `Error: unrecognized opcode csrw ... zicsr required` | GCC ≥ 12 without `_zicsr` | already handled; check that `riscv32-unknown-elf-gcc` is the compiler on PATH |
| Build errors in `qspi_micron.c` / `pwm_driver.c` (incompatible pointer) | GCC ≥ 14 makes these warnings errors | use your current toolchain, or add `-Wno-error=incompatible-pointer-types -Wno-error=int-conversion` |
| Program hangs at an XADC access | XADC not connected | don't use the XADC driver |
| Servo jitters / board resets | servo powered from the Nexys | external 5 V for the servo, common GND |
| gdb Ctrl-C shows `alpana_uart_set_baud` / a UART wait forever | `uart_struct` not volatile → `j .` at `-O2` (fixed in the SDK) | update `bsp/drivers/alpana/alpana_board.c`; in your own code use `ALPANA_UART0_*` / `alpana_uart_putb*` |
| Program hangs right after `load` only when using `ALPANA_FAST` code | boot RAM area overwritten / not copied | rebuild the whole SDK lib (`make software …` does it); as a test set `ALPANA_FASTRAM 0` in `alpana.h` |
| `ALPANA_FAST code does not fit in boot RAM` (link error) | more than 3968 B marked fast | mark only the innermost loops `ALPANA_FAST` |
| esp32cam_lane: yellow lines still missed | old sketch (v3, B&W) on the ESP32 | flash sketch v4 (banner "v4"); in the viewer inset yellow must look white |
| esp32cam_lane: beige / yellowish floor lights up in the inset | yellow boost too sensitive | raise `YEL_T` (e.g. 40) or lower `YEL_GAIN` in the sketch |

---

## 15. Known limitations
- The SPI1 driver is **receive-only**. The Shakti SPI transmit path gave wrong data in RTL simulation; use `gpio_spi.c` for writes.
- XADC and Ethernet are not available. `make upload` / `flash` (Arty spansion flow) is not adapted.
- The bus can hang if a program accesses an address that has no peripheral or memory. Stay inside the memory map.
- No hardware breakpoints (0 triggers).
- `printf` is the minimal Shakti version: no field width, no padding.

---

## 16. Changes compared with shakti-sdk
| File | Change |
|---|---|
| `Makefile` | `TARGET ?= alpana` |
| `software/examples/Makefile` | `alpana` target (rv32imafc/ilp32f, `-D ALPANA`), `_zicsr_zifencei` auto-detect, ALPANA drivers in `libshakti32.a`, auto-discovery of `PROGRAM` folders |
| `bsp/third_party/alpana/*` | new board: `platform.h` (80 MHz, 128 MB DDR2, boot RAM, CLINT÷256), `link.ld` (128 MB, `.text.*`, `.fasttext`/`.fastbss` in boot RAM), `ftdi.cfg` |
| `bsp/core/start.S` | FPU enable when built with F (`mstatus.FS`, `fcsr`) |
| `bsp/core/init.c` | copy `ALPANA_FAST` code to boot RAM; UART0 115200 before `main()`; park in boot RAM after `main()` (ALPANA only) |
| `bsp/include/alpana*.h`, `bsp/drivers/alpana/*` | new ALPANA drivers |
| `software/examples/alpana_applns/*` | new examples |
| `software/alpana_bench/*` | benchmark + stress suite |
| `alpana_tools/*` | OpenOCD config, gdb script, environment helper |
