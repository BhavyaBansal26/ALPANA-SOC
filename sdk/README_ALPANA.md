# ALPANA SDK

A fork of **shakti-sdk** set up for **ALPANA_SOC**: a Shakti E-class core made 4-stage, with a single-precision FPU (RV32IMAFC), running at 80 MHz on the Digilent Nexys 4 DDR with 128 MB of DDR2. All the stock Shakti targets and examples are still there. `alpana` is the default target.

> **New PC?** Start with `doc/ALPANA_Setup_Guide.md`. It covers Vivado + the SoC's Verilog/IP list, WSL, shakti-tools, usbipd and the first run. The board pinout is in `doc/ALPANA_SOC_Pinout.md`.

## Quick start (WSL)
```bash
cp -r /mnt/c/Users/Bhavya/ALPANA_SOC/ALPANA_SOC.srcs/alpana-sdk ~/alpana-sdk   # building on Linux FS is faster
cd ~/alpana-sdk && source alpana_tools/env.sh

make software PROGRAM=alpana_hello            # TARGET=alpana is the default
# terminal 1: OpenOCD     -> alpana-ocd          (or your existing working .cfg)
# terminal 2: UART        -> alpana-uart         (115200, /dev/ttyUSB1)
# terminal 3: load + run  -> alpana-run software/examples/alpana_applns/alpana_hello/output/alpana_hello.shakti
```
**Toolchain:** `riscv32-unknown-elf-gcc` with the rv32imafc/ilp32f multilib. The Makefile adds `_zicsr_zifencei` to `-march` automatically on GCC ≥ 12.

## What is different from shakti-sdk
| Part | ALPANA change |
|---|---|
| `TARGET=alpana` (default) | `-march=rv32imafc -mabi=ilp32f -D ALPANA` |
| `bsp/third_party/alpana/platform.h` | 80 MHz, 128 MB DDR2 @ 0x80000000, boot RAM 0x1000–0x2FFF, CLINT ÷256, same peripheral map as parashu |
| `bsp/third_party/alpana/link.ld` | 128 MB DDR2, stack/heap split 50/50, `.text.*` kept, `.fasttext`/`.fastbss` in on-chip boot RAM |
| `bsp/core/start.S` | FPU switched on (`mstatus.FS`) when built with F |
| `bsp/core/init.c` | copies `ALPANA_FAST` code into boot RAM; UART0 set to **115200** before `main()` (the boot ROM leaves another divisor); when `main()` returns, the core **parks in boot RAM** so the debugger can always halt it |
| `software/examples/Makefile` | any `<group>/<name>/<name>.c` builds with `PROGRAM=<name>`, no list entry needed |
| new drivers (in `libshakti32.a`) | `alpana.h` (board), `alpana_spi1.h`, `alpana_servo.h` (see below) |

## ALPANA drivers
All were derived from the ALPANA_SOC RTL. The SPI1 receive and PWM paths were checked in RTL simulation.

**`ALPANA_FAST`: hot code in on-chip RAM.** The core has no cache, so each instruction fetched from DDR2 costs about 48 cycles.
- Mark a function `ALPANA_FAST` and it runs from the free boot RAM (0x1C80–0x2BFF, 3.9 KB).
- `ALPANA_FAST_BSS` puts data in 0x2C10–0x2FBF (~0.9 KB).
- `init()` copies the code there before `main()`. The linker stops with an error if either area overflows.
- To turn it off, set `ALPANA_FASTRAM` to 0 in `bsp/include/alpana.h`.
- `alpana_uart_putb_fast()` is an inline raw UART write for such loops.

**`alpana.h`: board, GPIO, pin-mux, time**
- LEDs: `LED0..LED3` (GPIO16–19)
- Switches: `SW0..SW4` (GPIO 4, 7, 8, 14, 15)
- Buttons: `BTN_C/U/L/R` (GPIO20–23)
- Pmod JD: `JD1..JD10` (GPIO24–31)
- Functions:
  - GPIO: `alpana_gpio_output/input/set/clear/toggle/read/get`
  - pin-mux: `alpana_pinmux_set(PIN_JA7, PINMUX_ALT2)`
  - time: `alpana_cycles()`, `alpana_instret()`, `alpana_delay_us/ms()`, `alpana_millis()`
  - UART: `alpana_uart_putb()` (raw binary byte), `alpana_uart_set_baud()`
  - info: `alpana_banner()`

Pin-mux options (2 bits per pin):

| Pmod pin | 0 | 1 | 2 |
|---|---|---|---|
| JA1 | GPIO | UART1 RX | – |
| JA2 | GPIO | UART1 TX | – |
| JA3 | GPIO | UART2 RX | – |
| JA4 | GPIO | UART2 TX | PWM0 |
| JA7 | GPIO | – | **PWM1** |
| JA8 | GPIO | – | PWM2 |
| JA9 | GPIO | – | PWM3 |
| JA10 | GPIO | SPI1 NSS | PWM4 |
| JB1 | GPIO | SPI1 MOSI | PWM5 |
| JB2 | GPIO | **SPI1 MISO** | – |
| JB3 | GPIO | **SPI1 SCLK** | – |

**`alpana_spi1.h`: SPI1 master receive**
- Mode 3, 20 bytes per hardware burst. SCLK = 80 MHz / (2·(br+1)): br 7 → 5 MHz.
- `alpana_spi1_init(br)`, `alpana_spi1_read(buf, n)`, and `alpana_spi1_read_be_words()` (fastest).
- The next burst is started while the current one is being copied out (0 errors in RTL simulation).
- Use any GPIO as chip select.
- Transmit through this block is not supported. The block's TX path misbehaved in simulation, so use `gpio_spi.c` for writes.

**`alpana_servo.h`: PWM in µs and hobby servos**
- Functions:
  - `alpana_pwm_init_us(ch, period_us, high_us)`
  - `alpana_pwm_set_us()`
  - `alpana_servo_init(ch)` (50 Hz, 1500 µs)
  - `alpana_servo_set_angle(ch, deg, range)`
- Verified in simulation: 20.000 ms period / 1.500 ms pulse.
- PWM0..5 map to JA4, JA7, JA8, JA9, JA10, JB1.

## ALPANA examples (`software/examples/alpana_applns/`)
| PROGRAM | What it does |
|---|---|
| `alpana_hello` | banner, misa, mcycle vs CLINT timer check, FPU test, CPI of a loop |
| `alpana_blink` | LED chase on LD0–3; buttons pause/reverse; SW0 sets speed |
| `servo_sweep` | servo on JA7 sweeps ±30°; LEDs show direction |
| `esp32cam_lane` | ESP32-CAM → SPI1 → **lane detection on ALPANA (FPU)** → steering servo (JA7) + video with lane overlay on the PC. v2: colour camera + yellow-boost lane image, local-threshold detector with tracking, hot loops in boot RAM. Includes the ESP32 sketch (`esp32/`, v4), the PC viewer (`host/viewer.py`) and the offline test (`host/`). See its `README.md` |

## Benchmarks (`software/alpana_bench/`, standalone bare-metal)
```bash
cd software/alpana_bench && make          # build/perf.elf dhry.elf coremark.elf stress.elf
riscv32-unknown-elf-gdb -q -x run.gdb build/coremark.elf
```
These were measured on the board with code running from DDR2 and no cache:

| Benchmark | Result |
|---|---|
| Dhrystone | 0.015 DMIPS/MHz |
| CoreMark | 0.053 CoreMark/MHz |
| Stress | all pass |

Simulation (fast memory) gives 0.111 / 0.400. The gap is the uncached ~48-cycle DDR2 fetch.

## Adding your own application
```bash
mkdir -p software/examples/alpana_applns/myapp
cp software/examples/alpana_applns/alpana_hello/Makefile software/examples/alpana_applns/myapp/
# write software/examples/alpana_applns/myapp/myapp.c  (int main(void) { ... })
make software PROGRAM=myapp
```
For extra source files, set `EXTRA_SRC ?= ./a.c ./b.c` in the app Makefile. Apps build with `-O2`; change `OPT` to adjust.

## Notes
- The core has **no cache**. Instruction fetches from DDR2 cost about 48 cycles, so keep hot loops small. Use `-O2`, use word loads instead of byte loads where you can, and put the per-pixel/per-byte loops in `ALPANA_FAST`.
- `uart_struct` (stock `uart.h`) fields are not `volatile`. At `-O2`, a polling loop on `uart_instance[0]->status` compiles to an endless `j .`. Use the `ALPANA_UART0_*` macros or `alpana_uart_putb[_fast]()`.
- `alpana_tools/alpana_openocd.cfg` is the Shakti BSCAN/JTAG config (USER4, irlen 6). If your OpenOCD version wants the newer syntax, keep using the `.cfg` that already works for you.
- If the core can't be halted (`unable to halt`), it is busy fetching from DDR2. Reprogram the FPGA `.bit`, or press CPU RESET, then restart OpenOCD. Programs built with this SDK park in boot RAM when `main()` returns, which avoids this.
