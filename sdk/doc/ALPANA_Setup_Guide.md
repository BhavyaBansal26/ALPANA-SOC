# ALPANA: Setup Guide (hardware + SDK)

This guide takes a new PC to the point where:
1. the **ALPANA_SOC** bitstream is built and on the Nexys 4 DDR, and
2. the **ALPANA SDK** builds a program, loads it and prints on the UART.

It covers:
- the tools to install before the SDK;
- the full list of Verilog files and Xilinx IPs in the SoC;
- the first build/run check;
- the optional ESP32-CAM setup.

Related docs in `alpana-sdk/doc/`:
- `ALPANA_SDK_Guide.md`: SDK internals, drivers, examples;
- `ALPANA_SOC_Pinout.md`: board pinout.

---

## 0. What you need

| Item | Used for | Version used for ALPANA |
|---|---|---|
| Digilent **Nexys 4 DDR** (xc7a100t-csg324-1), micro-USB cable | the target board. One USB cable carries JTAG (programming + debug) and UART | Nexys 4 DDR / Nexys A7-100T |
| Windows 10/11 PC, 16 GB RAM recommended, ~120 GB free disk (Vivado is large) | Vivado + WSL | Windows 11 |
| **AMD Vivado ML** (Standard, free) | builds the bitstream | **2025.2** (the IPs were generated with it) |
| **WSL 2** + **Ubuntu 20.04** | SDK, toolchain, OpenOCD, gdb, Python | Ubuntu-20.04 |
| **usbipd-win** | gives WSL access to the Nexys USB (JTAG + UART) | latest |
| **shakti-tools** | `riscv32-unknown-elf-gcc/gdb` + Shakti **OpenOCD** | OpenOCD 0.10.0+dev-01252 (2020-09-22) |
| Python 3 + pyserial, numpy, opencv | camera viewer, lane test | Ubuntu 20.04 python3 |
| Arduino IDE 2.x + Espressif ESP32 core *(optional)* | ESP32-CAM sketch for `esp32cam_lane` | Arduino IDE 2.3.7 |

> **No `sudo`.** The commands below use `su -c '...'`, which asks for the root password. If your user is in the sudo group you may use `sudo` instead.

---

## Part A: Build the ALPANA_SOC bitstream (Windows, Vivado)

### A1. Install Vivado 2025.2
1. Download the **AMD Unified Installer** from the AMD/Xilinx download page. An AMD account is needed.
2. Choose **Vivado ML Standard**. Under devices, select **Artix-7**. You can deselect everything else to save disk.
3. Keep **"Install Cable Drivers"** ticked. This is needed for the Hardware Manager to see the board.
4. *(Recommended)* Install the Digilent board files:
   - download `vivado-boards` from github.com/Digilent/vivado-boards;
   - copy `new/board_files/*` to `<Vivado install>/2025.2/data/boards/board_files/`;
   - restart Vivado.

   Only the MIG's board flow uses them. The project itself uses the plain part.

> With a different Vivado version, run **Tools → Report → Report IP Status → Upgrade Selected** after opening the project. Then re-check the IP settings in A4, especially the MIG.

### A2. Project settings
| Setting | Value |
|---|---|
| Part | **xc7a100tcsg324-1** (Board: Nexys4 DDR, optional) |
| Top module | **`fpga_top`** |
| Target language | Verilog |
| Sources folder | `C:\Users\Bhavya\ALPANA_SOC\ALPANA_SOC.srcs\` (shortened to `srcs\` below) |

### A3. Verilog source files (54 files)
The SoC RTL is Bluespec-generated Verilog from the Shakti E-class (made 4-stage, with an SP FPU), plus the hand-written top level. This list is the full module hierarchy under `fpga_top`, worked out from the instantiations.

**Top level:** `srcs\sources_1\imports\Claude outputs\`

| File | Contents |
|---|---|
| `fpga_top.v` | board top: clocking, reset sync, `mkSoc_e32`, MIG + AXI clock converter, BSCANE2 JTAG, STARTUPE2 (QSPI clock), IOBUFs |

**SoC:** `srcs\sources_1\imports\verilog_soc_e32\`

| Group | Files |
|---|---|
| SoC + bus | `mkSoc_e32.v` |
| Core (E-class, 4-stage) | `mkriscv_e32.v`, `mkeclass_e32.v`, `mkstage0_e32.v`, `mkstage1_e32.v`, `mkstage2_e32.v`, `mkstage3_e32.v`, `mkalu_e32.v`, `mkmuldiv_e32.v`, `mkrestoring_div_e32.v`, `mkfpu_e32.v`, `mkcsr_e32.v`, `mkcsrfile_e32.v` |
| Core functions | `module_address_valid_e32.v`, `module_chk_interrupt_e32.v`, `module_decoder_func_32_e32.v`, `module_fn_alu_e32.v`, `module_fn_decompress_e32.v`, `module_singlestep_e32.v` |
| Debug | `mkriscvDebug013_e32.v` (RISC-V debug spec 0.13), `mkxilinxdtm_e32.v` (JTAG DTM over BSCANE2) |
| Interrupts | `mkplic_e32.v` |
| Peripherals | `mkuart_cluster_e32.v`, `mkuart_e32.v`, `mkspi_cluster_e32.v`, `mkspi_e32.v`, `mkpwm_cluster_e32.v`, `mkpwm_e32.v`, `mkmixed_cluster_e32.v`, `mkgpio_e32.v`, `mki2c_e32.v`, `mkpinmux_e32.v`, `mkpinmuxtop_e32.v` |
| Bluespec library | `BRAM2BELoad.v`, `ClockInverter.v`, `Counter.v`, `FIFO1.v`, `FIFO2.v`, `FIFO10.v`, `FIFO20.v`, `FIFOL1.v`, `MakeClock.v`, `MakeReset0.v`, `MakeResetA.v`, `RegFile.v`, `ResetEither.v`, `SizedFIFO.v`, `SyncFIFO1.v`, `SyncHandshake.v`, `SyncRegister.v`, `SyncReset0.v`, `SyncResetA.v`, `UngatedClockMux.v` |

**Memory initialisation file**

| File | Contents |
|---|---|
| `srcs\sources_1\imports\ecore\boot.mem` | boot ROM/RAM image (2048 × 32-bit words at 0x1000–0x2FFF). `BRAM2BELoad` loads it via `$readmemh("boot.mem")`, so it **must be in the project** (file type *Memory File*) |

**Files in the folder that are NOT used** (leave them out or disabled):

| File | Why |
|---|---|
| `bram_1rw.v`, `mkSencoder_e32.v`, `mkeclass_axi4lite_e32.v`, `module_decode_word32_e32.v` | not instantiated in this configuration |
| `Claude outputs\axi4_lutram.v`, `Downloads\axi4_lutram.v` | old on-chip-RAM build, replaced by DDR2 |
| `Claude outputs\fpga_top.v.*` (pre_ddr2, pre_lutram, lutram_version) | backups |
| `Claude outputs\multiplier_rtl.v` | plain-RTL **alternative** to the `multiplier` IP. It defines the **same module name**: use either this file or the IP, never both. The working build uses the IP |

**Constraints:** `srcs\constrs_1\new\`

| File | Contents |
|---|---|
| `constraints.xdc` | pins (clock E3, reset C12, UART C4/D4, QSPI, I2C, Pmod JA/JB/JD, switches, LEDs, buttons, interrupts), pull-ups/downs, JTAG `tck` clock, CDC false paths, `ASYNC_REG`/`MAX_FANOUT` timing fixes, 3.3 V config + 4-bit SPI bitstream settings |
| *(MIG generated)* | DDR2 pins and timing come from the MIG IP itself. **Do not** add `ddr2\nexys4ddr_ddr2_pins.xdc` to the project; it is only the MIG pin-import file |

### A4. Xilinx IPs (4) + primitives
All IP sources are in `srcs\sources_1\ip\<name>\<name>.xci`.

| IP (instance name) | Catalog IP | Configuration | Used by |
|---|---|---|---|
| **`clk_wiz_0`** | Clocking Wizard **6.0** | **PLL**, input 100 MHz (E3). **clk_out1 = 80 MHz** (core clock). **clk_out2 = 200 MHz** (MIG `sys_clk_i`). `reset` active high, `locked` output | `fpga_top` (`clk_div`) |
| **`mig_7series_0`** | Memory Interface Generator **(MIG 7 Series) 4.2** | see the MIG table below | `fpga_top` (`ddr2_ctrl`) |
| **`axi_clock_converter_0`** | AXI Clock Converter **2.1** | AXI4, data 32, address 32, **ID 4**, **asynchronous** clocks (core 80 MHz → MIG ui_clk) | `fpga_top` (`mem_cdc`) |
| **`multiplier`** | Multiplier (mult_gen) **12.0** | Parallel, A = B = **32-bit unsigned**, output [63:0], **Use Mults (DSP48)**, **2 pipeline stages** (must stay 2: `mkmuldiv_e32` samples at count 2) | `mkmuldiv_e32.v` (`mult`) |

**MIG (`mig_7series_0`) settings.** These are stored in `mig_a.prj` next to the `.xci`.

| Page | Setting |
|---|---|
| Controller | DDR2 SDRAM, 1 controller, **AXI4 interface** |
| Memory clock | **3333 ps (300 MHz)**, PHY:controller **4:1** → `ui_clk` = **75 MHz** |
| Memory part | **MT47H64M16HR-25E** (128 MB), data width **16**, data mask on, ordering Strict, 4 bank machines |
| Address map | BANK_ROW_COLUMN (row 13, col 10, bank 3) |
| AXI | data **32**, address **27**, ID **4** |
| Input clock | 200 MHz, **System clock: No Buffer** (from `clk_wiz_0` clk_out2) |
| Reference clock | **Use System Clock** |
| System reset | **ACTIVE LOW** |
| Vref | Internal Vref |
| Pins | Nexys 4 DDR DDR2 pinout (fixed pin-out; already in `mig_a.prj`) |

**Xilinx primitives** in `fpga_top.v` (no IP needed):
- `BSCANE2`: the JTAG tunnel to the debug module, used by OpenOCD;
- `STARTUPE2`: drives the QSPI flash clock (SPI0) through CCLK;
- `IOBUF`: GPIO, I2C and the pin-muxed pins.

### A5. Create the project (Tcl, optional)
In the Vivado Tcl console, adjust the two paths and run:
```tcl
set SRC  C:/Users/Bhavya/ALPANA_SOC/ALPANA_SOC.srcs
create_project ALPANA_SOC C:/work/ALPANA_SOC -part xc7a100tcsg324-1
# RTL
add_files [glob $SRC/sources_1/imports/verilog_soc_e32/*.v]
remove_files [list $SRC/sources_1/imports/verilog_soc_e32/bram_1rw.v \
                   $SRC/sources_1/imports/verilog_soc_e32/mkSencoder_e32.v \
                   $SRC/sources_1/imports/verilog_soc_e32/mkeclass_axi4lite_e32.v \
                   $SRC/sources_1/imports/verilog_soc_e32/module_decode_word32_e32.v]
add_files "$SRC/sources_1/imports/Claude outputs/fpga_top.v"
add_files $SRC/sources_1/imports/ecore/boot.mem
# IPs
foreach ip {clk_wiz_0 mig_7series_0 axi_clock_converter_0 multiplier} { add_files $SRC/sources_1/ip/$ip/$ip.xci }
upgrade_ip [get_ips]            ;# only does something if your Vivado is not 2025.2
generate_target all [get_ips]
# constraints + top
add_files -fileset constrs_1 $SRC/constrs_1/new/constraints.xdc
set_property top fpga_top [current_fileset]
update_compile_order -fileset sources_1
```
The existing project is `C:\Users\Bhavya\ALPANA_SOC\ALPANA_SOC.xpr`, and it already contains all of this.

### A6. Build and program
1. Run **Synthesis → Implementation → Generate Bitstream**.
2. Check timing in Design Runs: **WNS ≥ 0**. The timing-closed build had WNS +0.027 ns at 80 MHz.
   - If WNS < 0, keep the XDC timing-fix lines.
   - Try the implementation strategy *Performance_ExplorePostRoutePhysOpt*.
   - A negative WNS shows up as wrong `mul`/`div` results on the board.
3. Open **Hardware Manager → Open target → Auto connect**. Select `xc7a100t_0`, then **Program device** with `fpga_top.bit`.
4. Check that it came up:
   - LED/DONE is lit;
   - after the SDK setup (Part B), `alpana-ocd` finds the hart.

The bitstream is in RAM, so it is lost at power-off.
- To keep it, write it to the QSPI flash: Hardware Manager → Add Configuration Memory Device → s25fl128sxxxxxx0 (Nexys 4 DDR), with a `.bin` from *write_cfgmem*.
- Set the MODE jumper to **QSPI**.

**Close Hardware Manager before using OpenOCD.** Both need the same JTAG port.

---

## Part B: Software tools (WSL Ubuntu 20.04)

### B1. WSL 2 + Ubuntu 20.04
In **PowerShell (Administrator)**:
```powershell
wsl --install -d Ubuntu-20.04
wsl --set-default-version 2
wsl -l -v                      # Ubuntu-20.04 must show VERSION 2
```
Start Ubuntu once, then create your user and a root password. The root password is what `su` will ask for:
```powershell
wsl -d Ubuntu-20.04 -u root passwd root
```

### B2. Ubuntu packages
```bash
su -c 'apt-get update && apt-get install -y \
  build-essential make git curl wget zip unzip bc gawk bison flex texinfo gperf libtool patchutils \
  autoconf automake autotools-dev pkg-config device-tree-compiler zlib1g-dev libexpat1-dev \
  libmpc-dev libmpfr-dev libgmp-dev \
  libusb-1.0-0 libusb-1.0-0-dev libusb-0.1-4 libftdi1 libftdi1-2 libhidapi-hidraw0 \
  usbutils picocom telnet \
  python3 python3-pip python3-dev libpython3.8-dev'
```
- **Toolchain and OpenOCD libraries:** `libusb-1.0-0`, `libftdi1-2`, `libmpc/mpfr/gmp`, `libexpat`, `libpython3.8`. The `-dev` packages are the list from the Shakti instructions.
- **USB and debug utilities:** `usbutils` (`lsusb`), `picocom` (UART terminal), `telnet` (OpenOCD console).

### B3. shakti-tools: RISC-V GCC/GDB + OpenOCD
```bash
cd ~
git clone --recursive https://gitlab.com/shaktiproject/software/shakti-tools.git   # large, takes a while
```
Add to the end of `~/.bashrc`. Adjust `SHAKTITOOLS` if you cloned it somewhere else:
```bash
SHAKTITOOLS=$HOME/shakti-tools
export PATH=$PATH:$SHAKTITOOLS/bin
export PATH=$PATH:$SHAKTITOOLS/riscv32/bin:$SHAKTITOOLS/riscv32/riscv32-unknown-elf/bin
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$SHAKTITOOLS/riscv32/lib
# (optional 64-bit toolchain)
export PATH=$PATH:$SHAKTITOOLS/riscv64/bin
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$SHAKTITOOLS/riscv64/lib
```
Check it in a **new** terminal:
```bash
which riscv32-unknown-elf-gcc riscv32-unknown-elf-gdb openocd
riscv32-unknown-elf-gcc --version
riscv32-unknown-elf-gcc -print-multi-lib | grep rv32imafc     # must print rv32imafc/ilp32f
openocd --version                                           # Open On-Chip Debugger 0.10.0+dev-01252 ...
```
ALPANA needs a toolchain with the **rv32imafc / ilp32f** multilib (because of the FPU).
- GCC ≥ 12 also works; the SDK adds `_zicsr_zifencei` itself.
- Use the **shakti-tools OpenOCD**. It has the Shakti RISC-V target and works with `alpana_openocd.cfg`.

### B4. Python packages
These are needed for the camera viewer and the lane test.
```bash
python3 -m pip install --user --upgrade pip
python3 -m pip install --user pyserial numpy opencv-python
```
The viewer opens a window under WSLg (Windows 11). Without a display it saves `latest.jpg` instead (`--save DIR`).

### B5. usbipd-win (Windows): connect the Nexys USB to WSL
In **PowerShell (Administrator)**:
```powershell
winget install --interactive --exact dorssel.usbipd-win
# reboot or re-open PowerShell, plug in the Nexys (power switch ON)
usbipd list
```
Find the Nexys: **VID:PID `0403:6010`** ("USB Serial Converter A, B"). An ESP32-CAM-MB shows up as a different device (0403:6001 or 1a86:7523); leave that one in Windows for the Arduino IDE.
```powershell
usbipd bind   --busid <BUSID>          # once per device/port (Administrator)
usbipd attach --wsl --busid <BUSID>    # after every replug / reboot / WSL restart
```
The bus id can change when you use a different USB port (for example 1-5 → 2-5). Run `usbipd list` again if `attach` fails.

Then in WSL, after **every** `attach`, give your user access:
```bash
lsusb | grep 0403:6010                      # Future Technology Devices ... FT2232C/D/H
su -c 'chmod 666 /dev/bus/usb/*/*'          # fixes LIBUSB_ERROR_ACCESS
ls /dev/ttyUSB*                             # ttyUSB0 = JTAG channel, ttyUSB1 = ALPANA UART
```
To make the permission permanent, try a udev rule. Udev does not always run under WSL; if it doesn't take effect, keep using the `chmod` line.
```bash
su -c 'echo SUBSYSTEM==\"usb\", ATTR{idVendor}==\"0403\", ATTR{idProduct}==\"6010\", MODE=\"0666\" > /etc/udev/rules.d/99-nexys-ftdi.rules; service udev restart; udevadm control --reload-rules; udevadm trigger'
```

---

## Part C: Install the ALPANA SDK

### C1. Copy the SDK into the Linux file system
Building from `/mnt/c` is slow, so copy it into your Linux home:
```bash
cp -r /mnt/c/Users/Bhavya/ALPANA_SOC/ALPANA_SOC.srcs/alpana-sdk ~/alpana-sdk
# or:  unzip /mnt/c/.../alpana-sdk.zip -d ~
```
To update it later, copy the changed folders over the old ones:
```bash
cp -r /mnt/c/Users/Bhavya/ALPANA_SOC/ALPANA_SOC.srcs/alpana-sdk/{bsp,software,doc,alpana_tools,README_ALPANA.md} ~/alpana-sdk/
```

### C2. Environment
```bash
echo 'source ~/alpana-sdk/alpana_tools/env.sh' >> ~/.bashrc
source ~/.bashrc
```
This sets `ALPANASDK` (and `SHAKTISDK` for the stock Makefiles) and three helpers:

| Helper | What it does |
|---|---|
| `alpana-ocd` | OpenOCD with `alpana_tools/alpana_openocd.cfg` (FT2232 channel A, BSCANE2 / USER4) |
| `alpana-run <file.shakti>` | gdb: connect to :3333, halt, load, `pc = _start`, continue |
| `alpana-uart` | picocom on `/dev/ttyUSB1` at 115200 |

### C3. First build
```bash
cd ~/alpana-sdk
make software PROGRAM=alpana_hello          # TARGET=alpana is the default
# -> software/examples/alpana_applns/alpana_hello/output/alpana_hello.shakti
```
Other examples, all in `software/examples/alpana_applns/`:
- `alpana_blink`
- `servo_sweep`
- `esp32cam_lane`

The benchmarks are built separately:
```bash
cd software/alpana_bench && make
```

### C4. First run (3 terminals)
1. Program the FPGA with `fpga_top.bit` (A6), then **close Vivado Hardware Manager**.
2. Attach the USB (B5), then run the `chmod` line.

| Terminal | Command | Expected |
|---|---|---|
| 1 | `alpana-ocd` | `Info : ... hart 0 ... ` and `Listening on port 3333 for gdb connections` |
| 2 | `alpana-uart` | (waits) |
| 3 | `alpana-run software/examples/alpana_applns/alpana_hello/output/alpana_hello.shakti` | gdb loads the program and continues |

Terminal 2 then shows the `ALPANA_SOC (Shakti E-class 4-stage + SP FPU) RV32ACFIMSU @ 80 MHz DDR2 128 MB` banner, the timer and FPU checks, and CPI.

When `main()` returns, the program **parks in boot RAM**. You can then load the next one directly: press Ctrl-C in gdb, `quit`, and run `alpana-run` again.

### C5. Setup checklist
| Check | Command | Good result |
|---|---|---|
| WSL 2 | `wsl -l -v` (PowerShell) | Ubuntu-20.04, VERSION 2 |
| toolchain | `riscv32-unknown-elf-gcc -print-multi-lib \| grep rv32imafc` | `rv32imafc/ilp32f;...` |
| OpenOCD | `openocd --version` | shakti build, 0.10.0+dev |
| USB in WSL | `lsusb \| grep 0403:6010` | FT2232 listed |
| permissions | `alpana-ocd` | no `LIBUSB_ERROR_ACCESS` |
| UART | `ls /dev/ttyUSB1` | exists |
| SDK | `make software PROGRAM=alpana_hello` | `built ./output/alpana_hello.shakti` |
| board | `alpana-run …alpana_hello.shakti` | banner on the UART |
| DDR2 | `telnet localhost 4444`, then `mww 0x80000000 0x12345678` and `mdw 0x80000000` | `12345678` |

---

## Part D: ESP32-CAM (only for `esp32cam_lane`)

1. Install **Arduino IDE 2.x** on Windows.
2. **File → Preferences → Additional boards manager URLs:**
   `https://espressif.github.io/arduino-esp32/package_esp32_index.json`
3. **Boards Manager:** install **"esp32 by Espressif Systems"**.
4. **Tools:**
   - Board → **AI Thinker ESP32-CAM**;
   - PSRAM **Enabled** (default for this board);
   - Port → the ESP32-CAM-MB COM port.
5. USB driver: the ESP32-CAM-MB uses a **CH340**. If no COM port appears, install the WCH CH340 driver.
6. Open the **SDK copy** of the sketch: `alpana-sdk\software\examples\alpana_applns\esp32cam_lane\esp32\esp32cam_spi_slave\esp32cam_spi_slave.ino`. The Serial Monitor banner must say **v4**.
7. **Unplug the IO2 (CS) wire** before uploading, and plug it back afterwards. Then press RST.
8. Wiring: see `software/examples/alpana_applns/esp32cam_lane/WIRING.md`.
9. Run: `python3 viewer.py /dev/ttyUSB1` (in `esp32cam_lane/host`). Then `alpana-run …/esp32cam_lane.shakti`, then RST on the ESP32.

---

## Part E: Setup troubleshooting
| Symptom | Fix |
|---|---|
| `usbipd: error: Device with busid 'x-y' is already attached` | It is already in WSL; just run the `chmod` line |
| `usbipd attach` cannot find the busid | Re-run `usbipd list`; the busid changes with the USB port. `bind` it again (Administrator) |
| `LIBUSB_ERROR_ACCESS` / `unable to open ftdi device` | Run `su -c 'chmod 666 /dev/bus/usb/*/*'` after every attach |
| `su: openocd: command not found` | Don't run OpenOCD as root. Root's PATH does not have shakti-tools; use `alpana-ocd` as your user |
| OpenOCD: `Error: JTAG scan chain interrogation failed` | Vivado Hardware Manager is still connected (close it), or the FPGA is not programmed |
| `unable to halt hart 0` | A program is running from DDR2. Press CPU RESET (or reprogram the `.bit`), restart OpenOCD and load again |
| `Error: unrecognized opcode csrw ... zicsr` | A GCC ≥ 12 toolchain is on PATH: fine, the SDK handles it. Make sure the `riscv32-unknown-elf-gcc` found by `which` is the one you expect |
| `cannot find -lc` or no `rv32imafc` multilib | The toolchain lacks the F multilib; use the shakti-tools riscv32 toolchain |
| Vivado: IPs locked / out of date | Report IP Status → Upgrade, then *Generate Output Products*, then reset `synth_1` |
| Vivado: multiple definitions of module `multiplier` | Remove `multiplier_rtl.v` (or the `multiplier` IP); keep one |
| Core never comes out of reset (OpenOCD can't examine) | DDR2 calibration failed: check the MIG settings (A4) and that `clk_wiz_0` clk_out2 is 200 MHz |
| Wrong multiply/divide results | Timing not met (WNS < 0); see A6 |

---

## Appendix: Folder map (`ALPANA_SOC.srcs`)
```
ALPANA_SOC.srcs/
├── sources_1/
│   ├── imports/verilog_soc_e32/   SoC RTL (A3)
│   ├── imports/Claude outputs/    fpga_top.v (+ backups, unused alternatives)
│   ├── imports/ecore/boot.mem     boot ROM image
│   └── ip/                        clk_wiz_0, mig_7series_0 (+ mig_a.prj), axi_clock_converter_0, multiplier
├── constrs_1/new/constraints.xdc
├── ddr2/                          DDR2 integration notes, IP Tcl, MIG pin file (reference only)
├── alpana-sdk/                    the SDK (Part C) + doc/
├── bench_nexys/, alpana_cam/      earlier stand-alone benchmark / camera builds
└── _backup_pre_*/                 backups from the timing / bug fixes
```
