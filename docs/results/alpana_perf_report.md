# ALPANA_SOC: performance report

**Stock:** original Shakti E-class (3-stage, RV32IMAC, no FPU) at **50 MHz**.
**ALPANA:** 4-stage core with the single-precision FPU, timing-fixed, at **80 MHz**.

All numbers come from cycle-accurate RTL simulation on the same SoC testbench, running the same `-O2` binaries. They are scaled to each core's clock. Board results with DDR3 will be lower in absolute terms, but the ratios should hold.

## Scores

| | Stock @ 50 MHz | ALPANA @ 80 MHz | Gain |
|---|---|---|---|
| DMIPS/MHz | 0.114 | 0.108 | 0.95× |
| **DMIPS** | 5.72 | **8.64** | **1.51×** |
| CoreMark/MHz | 0.423 | 0.403 | 0.95× |
| **CoreMark** | 21.1 | **32.2** | **1.53×** |

## CPI and MIPS

| Workload | Stock CPI | ALPANA CPI | Stock MIPS | ALPANA MIPS |
|---|---|---|---|---|
| Dhrystone | 9.15 | 9.69 | 5.5 | **8.3** |
| CoreMark | 7.55 | 7.93 | 6.6 | **10.1** |
| Integer kernels (matmul, sort, CRC32) | 6.42 | 6.43 | 7.8 | **12.4** |

The extra pipeline stage costs about 5 % in CPI and buys 1.6× in clock. The net result is about 1.5× more MIPS.

## Hardware FPU vs soft-float

Benchmark: 16×16 fp32 matrix multiply, 8192 FLOPs.

| | Instructions | Cycles | Time | MFLOPS |
|---|---|---|---|---|
| Stock, soft-float @ 50 MHz | ~851 k | 5.61 M | 112 ms | 0.07 |
| ALPANA, soft-float @ 80 MHz | 851 k | 5.64 M | 70.5 ms | 0.12 |
| **ALPANA, hardware FPU @ 80 MHz** | **35.6 k** | **0.32 M** | **3.95 ms** | **2.07** |

- On the same core, the FPU runs **24× fewer instructions** and is **17.8× faster**.
- Against the stock core at its 50 MHz, ALPANA is **28× faster** on floating point.

FPU instruction cost in cycles: fadd / fmul / fmadd 9, fdiv 19, fsqrt 30, fcvt / fmin / fsgnj / feq 6. On the stock core, each of these is a software library call.

## Cost

| | Stock | ALPANA |
|---|---|---|
| Core LUT / FF / DSP (Yosys estimate) | 3.5 k / 1.4 k / 4 | 9.2 k / 2.9 k / 6 |
| Clock on the Arty A7 | 50 MHz | 80 MHz (timing met) |

## Bottleneck

Both cores spend about 6 cycles on every 32-bit instruction fetch, because there is no instruction cache. Running code from on-chip BRAM or adding a small I-cache is the next big improvement.
