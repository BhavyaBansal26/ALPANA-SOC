# ALPANA_SOC — Board Results (Nexys 4 DDR, xc7a100t)

**Config:** Shakti E-class 4-stage, RV32IMAFC + SP FPU, 80 MHz core clock.
Main memory is 128 MB DDR2 through the MIG (AXI4, 32-bit) and an async clock converter. **No cache.**
Implementation: WNS +0.027 ns, WHS +0.008 ns (timing met).

## Functional verification — ALL PASS
| Test | Result |
|---|---|
| mul/mulhu/mulh/div/divu/remu (mdtest2, single-step) | 8/8 correct |
| perf suite (ISA latency, memory, traps, 4 kernels) | PASS, all checksums correct |
| Dhrystone 2.1 (20000 runs) | PASS (all variables match) |
| CoreMark 1.0 (400 iter, 94 s) | PASS, CRCs validated |
| Stress [A] random ALU/FPU/ctrl kernels vs host golden | 6/6 match |
| Stress [B] DDR2 memory test, 1 MB (March C-, addr-in-addr, byte/half lanes) | 0 errors |
| Stress [C] AMO / LR/SC / CSRs | 0 errors |
| Stress [D] interrupt storm (~21k timer + SW IRQs) | 0 mismatches |
| Stress [E] 10-round soak under interrupt load | 0 errors |

## Performance
| Metric | Board (DDR2, no cache) | RTL sim | Board / sim |
|---|---|---|---|
| Dhrystone CPI | 69.6 | 9.4 | 7.4x |
| DMIPS | 1.20 | 8.90 | |
| DMIPS/MHz | 0.015 | 0.111 | |
| CoreMark CPI | 60.0 | 8.0 | 7.5x |
| CoreMark iter/s | 4.26 | 31.98 | |
| CoreMark/MHz | 0.053 | 0.400 | |
| perf whole-suite CPI | 55.0 | | |

Note: the stock CoreMark header reports "Total time 40 s / 10 iter/s". Its 32-bit tick counter wrapped, so those figures are wrong. Use the 64-bit mcycle figures above (7.52 G cycles = 94 s).

## Why the board is ~7.5x slower than simulation
The E-class core has no cache. Every instruction fetch is a single 32-bit read through the AXI clock converter, the MIG and DDR2, at about 600 ns (about **48 core cycles**). The perf suite shows this directly:
- any simple instruction (add, mul, fadd, fdiv): 48 cycles, i.e. 1 fetch
- RVC instruction: 24 cycles (2 instructions per fetch)
- lw/sw: 96 cycles (fetch + data)
- taken branch: 144 cycles (refetch)

The core and FPU are not the bottleneck; memory latency is.
**Fix:** a small RTL cache (8–16 KB) between mem_master and DDR2, or code in BRAM. Either should bring the CPI back near the simulation numbers.

## Bugs found and fixed during bring-up
1. **Multiplier returned 0 on hardware.** Root cause: timing failure (WNS −2.756 ns). Fixed with three changes:
   - a false path on the MIG init_calib_complete → rst_sync CDC;
   - MAX_FANOUT on soc_reset;
   - the Performance_ExplorePostRoutePhysOpt impl strategy.
2. **Multiplier IP misconfigured.** It was 16x16 signed with 3 stages; it is now 32x32 unsigned with 2 stages (= MULSTAGES).
3. **MIG AXI width was 128 bits** with narrow burst off, so words aliased. Fixed by setting it to 32-bit with narrow burst on.
4. **Debugger could not halt the core while it was fetching from DDR2.** Workaround: test programs park in boot RAM (0x2C00) at exit.
