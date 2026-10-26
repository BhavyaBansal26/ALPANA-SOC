# ALPANA_SOC: DDR2 main memory on the Nexys4 DDR

## Why
The SoC sends every main-memory access (0x8000_0000–0x8FFF_FFFF) out of its `mem_master` AXI4 port. In the current `fpga_top.v` that port is **not connected**, so the first load to DDR waits forever. That is the 175-second OpenOCD stall.

Using a standard AXI4 port plus a vendor memory controller keeps ALPANA portable. On another FPGA you only swap the memory-controller IP; the SoC and the SDK do not change.

```
SoC mem_master (AXI4, 80 MHz) -> axi_clock_converter_0 -> mig_7series_0 (AXI4 slave) -> DDR2 128 MB
                                  core_clk -> ui_clk          ui_clk = 81.25 MHz
reset to SoC = PLL locked AND init_calib_complete (synchronised to core_clk)
```

## Steps (Vivado)

### 1. Install the Digilent board files once
These give the MIG the exact DDR2 pinout.
1. Download `vivado-boards` from github.com/Digilent/vivado-boards.
2. Copy `new/board_files/*` into `<Vivado>/data/boards/board_files/`.
3. Restart Vivado.
4. Go to Project Settings → General → **Project device: Board → Nexys4 DDR**. The part number stays xc7a100tcsg324-1.

### 2. Clock wizard and clock converter
In the Tcl console:
```
source C:/Users/Bhavya/ALPANA_SOC/ALPANA_SOC.srcs/ddr2/add_ddr2_ips.tcl
```

### 3. Create the MIG
Go to IP Catalog → **Memory Interface Generator (MIG 7 Series)**. Name it `mig_7series_0`.

| Page | Setting |
|---|---|
| Board tab (if offered) | DDR2 SDRAM → Nexys4 DDR interface (fills the pins) |
| Controller | **AXI4 interface**, DDR2 SDRAM, 1 controller |
| Clock period | **3077 ps (325 MHz)**, PHY to controller ratio **4:1** |
| Memory part | **MT47H64M16HR-25E**, data width **16**, ordering Strict |
| AXI parameters | data width **32**, **ID width 4**, address read/write arbitration Round Robin |
| Input clock period | **5000 ps (200 MHz)** |
| System clock | **No Buffer** |
| Reference clock | **Use System Clock** |
| System reset polarity | **ACTIVE LOW** |
| Internal Vref | **enabled** |
| XADC instantiation | Enabled (the SoC's own XADC is removed in the Nexys build) |
| Pin selection | **Fixed Pin Out** → *Read XDC/UCF* using the Digilent `Nexys4DDR` MIG prj/UCF, **or** let the board flow fill it |

Click Generate.

### 4. Patch the top level
In WSL:
```
cd "/mnt/c/Users/Bhavya/ALPANA_SOC/ALPANA_SOC.srcs/sources_1/imports/Claude outputs"
python3 /mnt/c/Users/Bhavya/ALPANA_SOC/ALPANA_SOC.srcs/ddr2/patch_top_ddr2.py fpga_top.v
```
- The original is kept as `fpga_top.v.pre_ddr2`.
- **No XDC change is needed for DDR2.** The MIG generates its own pin constraints.

### 5. Build
```
reset_run synth_1
launch_runs impl_1 -to_step write_bitstream -jobs 8
```
Check timing:
- `core_clk` must still meet 80 MHz.
- The MIG `ui_clk` (81.25 MHz) paths are internal to Xilinx IP and normally close.
- The clock converter declares the CDC to Vivado itself, so no extra constraints are needed.

## SDK linker script (128 MB)
In `bsp/third_party/alpana/link.ld`, change the main memory length to **128M** (Parashu's is 256M on the Arty). Put the stack top at or below `0x87FFFFFF`.

## Board bring-up check
```
telnet localhost 4444
mww 0x80000000 0x12345678
mdw 0x80000000 1        # -> 12345678  (DDR2 works)
```
If the core never comes out of reset, DDR2 calibration failed. Check `init_calib_complete` on an LED, or check the MIG clock settings above.
