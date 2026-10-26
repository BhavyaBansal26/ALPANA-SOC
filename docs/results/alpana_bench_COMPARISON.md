# ALPANA_SOC vs the original Shakti E-class SoC

All three cores ran the same benchmark binaries on the same Verilator SoC testbench (same AXI fabric, memory model, UART and CLINT). That makes the cycle counts directly comparable.

| Label | Core |
|---|---|
| **Stock** | Original Shakti E-class: 3-stage, RV32IMAC, no FPU |
| **ALPANA-pre** | Your 4-stage core with the SP FPU, before the timing fixes |
| **ALPANA** | Your 4-stage core with the SP FPU, after the 80 MHz timing fixes and the two bug fixes (the version in `rtl_fix/`) |

The integer benchmarks use one binary built for RV32IMAC with `-O2`, so all three cores ran identical code. ALPANA was also run with an RV32IMAFC build to measure its hard FPU.

## Headline

| Metric | Stock | ALPANA-pre | ALPANA | ALPANA vs stock |
|---|---|---|---|---|
| Dhrystone cycles/run | 4977 | 5262 | 5271 | +5.9 % |
| **DMIPS/MHz** | **0.114** | 0.108 | **0.108** | −5.6 % |
| CoreMark cycles/iter | 2.367 M | 2.441 M | 2.484 M | +5.0 % |
| **CoreMark/MHz** | **0.423** | 0.410 | **0.403** | −4.7 % |
| fp32 matmul 16×16 (cycles) | 5.61 M (soft-float) | — | **316 k (hard FPU)** | **17.7× faster** |
| MFLOPS @ 80 MHz | 0.12 | — | **2.07** | 17× |
| Integer kernel mix CPI | 6.5 | — | 6.5 | same |
| Core area: LUT / FF / DSP (Yosys, xc7, core only) | 3.5 k / 1.4 k / 4 | 9.5 k / 2.7 k / 6 | 9.2 k / 2.9 k / 6 | ~2.6× LUT |
| `minstret` works | **no** (original bug) | no | **yes** | fixed |
| Proven Fmax on the Arty | not measured | ≈70 MHz (WNS −4.3 ns at 100 MHz) | **80 MHz, timing met** | — |

## Instruction costs (cycles, loop-aligned, from the `perf` micro-benchmarks)

| Instruction | Stock | ALPANA |
|---|---|---|
| nop / add / mul / branch not-taken | 6 | 6 |
| c.add (RVC) | 3 | 3 |
| lw load-to-use | 12 | 12 |
| sw | 10 | 10 |
| taken branch / jal | 18 | 18 |
| taken branch to a target at pc[1]=1 | 21 | 21 |
| div / rem | 33 | 34 |
| csrr / fence | 23 / 24 | 23 / 24 |
| ecall → handler | 63 | 63 |
| software IRQ → handler | 81 | 76 |
| fadd / fmul / fmadd | soft-float library call | **9** |
| fdiv / fsqrt | soft-float | **19 / 30** |

## What this means

1. **Per instruction, the two cores are the same, because both are limited by instruction fetch.**
   - Every 32-bit fetch is a full AXI transaction of about 6 cycles. The pipeline depth (3 vs 4 stages) hardly matters, which is why `nop`, `add` and `mul` all cost 6 cycles on both.
2. **ALPANA is about 5 % slower per MHz on integer code.** Three causes:
   - The extra pipeline stage adds a cycle to taken-branch and redirect paths.
   - The timing fixes hold multiply/divide and FPU results for one extra cycle before writeback.
   - The registered branch redirect adds a cycle.

   ALPANA-pre → ALPANA (the timing fixes alone) cost about 1–2 % on CoreMark and Dhrystone.
3. **On floating-point code, ALPANA is about 17× faster.** The stock core has to emulate FP in software.
4. **ALPANA's real advantage is clock speed and FP.**
   - It is proven at 80 MHz on the Arty. I haven't measured the stock SoC's Fmax in Vivado: if it closes at 80 MHz or higher, it also wins on integer DMIPS.
   - For FP work, ALPANA wins outright.
5. **Both cores had the `minstret` bug.** It is in upstream Shakti E-class, not in your changes: the counter reads 0 on the stock core too. The fix in `rtl_fix/` would also apply to stock Shakti.
6. **Stock stress-test result:** the integer kernels, the memory test, the atomics and the 35,806-interrupt storm all passed. It reported 2 errors:
   - `minstret` reads 0: the real upstream bug.
   - My test expected the F bit in `misa`: a test bug, now fixed in `stress.c`.
7. **Biggest speed-up for either core:** remove the fetch bottleneck. Running code from on-chip BRAM (a TCM) or adding a small I-cache should cut CPI by much more than the pipeline depth or the FPU affects it.

## Caveats

- These are RTL-simulation results with the testbench memory, not board results. On the Arty, code runs from DDR3 through the MIG, so absolute CPI will be worse for both cores. The relative comparison should still hold.
- The area numbers are Yosys core-only estimates. Vivado will be lower, but the ratio is representative.
- The Dhrystone run is short (200 runs) and CoreMark ran 1 iteration, so the CoreMark score is not an official one. The CRCs were validated on all three cores.
