/*
 * ALPANA_SOC performance characterisation
 *   1. instruction latency / throughput micro-benchmarks (cycles per instruction)
 *   2. memory system: load-to-use, pointer chase, read/write/copy bandwidth
 *   3. trap & interrupt latency
 *   4. application kernels: CPI, IPC, MIPS, MFLOPS
 * All timing uses mcycle/minstret (64-bit), so the numbers are exact cycles.
 */
#include "lib.h"

#ifdef __riscv_flen
#define ISA_STR "RV32IMAFC (hard FPU)"
#else
#define ISA_STR "RV32IMAC (soft-float)"
#endif
#ifndef LOOPS
#define LOOPS 20
#endif

/* ------------------------------------------------------------------ */
/* 1. micro-benchmarks                                                 */
/* Each test is run with the body repeated 32x and 64x per loop trip;  */
/* (c64 - c32) / (32 * K * LOOPS) removes loop overhead exactly.       */
/* ------------------------------------------------------------------ */
#ifdef __riscv_flen
#define CLOB "t0","t1","t2","t3","t4","t5","t6","a2","a3","a4","a5", \
             "ft0","ft1","ft2","ft3","ft4","ft5","memory"
#else
#define CLOB "t0","t1","t2","t3","t4","t5","t6","a2","a3","a4","a5","memory"
#endif

static uint32_t scratch[64] __attribute__((aligned(64)));

#define UB_BODY(N, setup, insn)                                        \
  asm volatile(".option push\n.option norvc\n"                         \
               "mv a5, %[p]\n" setup "\n"                              \
               "li t6, %[L]\n"                                         \
               ".option rvc\n.balign 4\n.option norvc\n"               \
               "1:\n.rept " #N "\n" insn "\n.endr\n"                   \
               "addi t6, t6, -1\n bnez t6, 1b\n"                       \
               "9:\n.option pop\n"                                     \
               :: [p] "r"(scratch), [L] "i"(LOOPS) : CLOB)

#define DEF_UB(name, setup, insn)                                      \
  static __attribute__((noinline)) uint64_t name##_32(void) {         \
    uint64_t c = rdcycle64(); UB_BODY(32, setup, insn);                \
    return rdcycle64() - c; }                                          \
  static __attribute__((noinline)) uint64_t name##_64(void) {         \
    uint64_t c = rdcycle64(); UB_BODY(64, setup, insn);                \
    return rdcycle64() - c; }

#define ONE "li t0,1\n li t1,3\n li t2,5\n"
#define F1  "li t0,0x3f800000\n fmv.w.x ft0,t0\n fmv.w.x ft1,t0\n fmv.w.x ft2,t0\n fmv.w.x ft3,t0\n"

DEF_UB(u_nop,      "",  "nop")
DEF_UB(u_add_dep,  ONE, "add t0,t0,t1")
DEF_UB(u_add_ind,  ONE, "add t3,t1,t2\n add t4,t1,t2\n add t5,t1,t2\n add a2,t1,t2")
DEF_UB(u_c_add,    ONE "li a2,1\n li a3,2\n", ".option push\n.option rvc\nc.add a2,a3\n.option pop")
DEF_UB(u_lw_ind,   "",  "lw t0,0(a5)\n lw t1,4(a5)\n lw t2,8(a5)\n lw t3,12(a5)")
DEF_UB(u_lw_use,   "",  "lw t0,0(a5)\n add t1,t0,t0")
DEF_UB(u_lw_chase, "sw a5,0(a5)\n mv t0,a5\n", "lw t0,0(t0)")
DEF_UB(u_sw,       ONE, "sw t1,0(a5)\n sw t1,4(a5)\n sw t1,8(a5)\n sw t1,12(a5)")
DEF_UB(u_st_ld,    ONE, "sw t1,0(a5)\n lw t2,0(a5)")
DEF_UB(u_br_nt,    ONE, "beqz t0, 9f")
DEF_UB(u_br_t,     "",  "beq x0,x0,2f\n2:")
DEF_UB(u_br_t_mis, "",  "beq x0,x0,2f\n .option rvc\n c.nop\n .option norvc\n2:")
DEF_UB(u_jal,      "",  "jal x0,2f\n2:")
DEF_UB(u_jalr,     "",  "auipc t0,0\n jalr x0,8(t0)")
DEF_UB(u_mul_dep,  ONE, "mul t0,t0,t1")
DEF_UB(u_mul_ind,  ONE, "mul t3,t1,t2\n mul t4,t1,t2\n mul t5,t1,t2\n mul a2,t1,t2")
DEF_UB(u_mulh_dep, ONE, "mulh t0,t0,t1")
DEF_UB(u_div_dep,  "li t0,0x7fffffff\n li t1,1\n", "div t0,t0,t1")
DEF_UB(u_div_ind,  "li t1,0x7fffffff\n li t2,7\n", "div t3,t1,t2\n div t4,t1,t2")
DEF_UB(u_rem_dep,  "li t0,0x7fffffff\n li t1,0x7ffffffe\n", "remu t0,t0,t1")
#ifdef __riscv_flen
DEF_UB(u_fadd_dep, F1,  "fadd.s ft0,ft0,ft1")
#endif
#ifdef __riscv_flen
DEF_UB(u_fadd_ind, F1,  "fadd.s ft2,ft0,ft1\n fadd.s ft3,ft0,ft1\n fadd.s ft4,ft0,ft1\n fadd.s ft5,ft0,ft1")
#endif
#ifdef __riscv_flen
DEF_UB(u_fmul_dep, F1,  "fmul.s ft0,ft0,ft1")
#endif
#ifdef __riscv_flen
DEF_UB(u_fmul_ind, F1,  "fmul.s ft2,ft0,ft1\n fmul.s ft3,ft0,ft1\n fmul.s ft4,ft0,ft1\n fmul.s ft5,ft0,ft1")
#endif
#ifdef __riscv_flen
DEF_UB(u_fmadd_dep,F1,  "fmadd.s ft0,ft0,ft1,ft1")
#endif
#ifdef __riscv_flen
DEF_UB(u_fdiv_dep, F1,  "fdiv.s ft0,ft0,ft1")
#endif
#ifdef __riscv_flen
DEF_UB(u_fsqrt_dep,F1,  "fsqrt.s ft0,ft0")
#endif
#ifdef __riscv_flen
DEF_UB(u_fcmp,     F1,  "feq.s t0,ft0,ft1")
#endif
#ifdef __riscv_flen
DEF_UB(u_fminmax,  F1,  "fmin.s ft0,ft0,ft1")
#endif
#ifdef __riscv_flen
DEF_UB(u_fsgnj,    F1,  "fsgnj.s ft0,ft0,ft1")
#endif
#ifdef __riscv_flen
DEF_UB(u_fcvt_sw,  ONE, "fcvt.s.w ft0,t1")
#endif
#ifdef __riscv_flen
DEF_UB(u_fcvt_ws,  F1,  "fcvt.w.s t0,ft0")
#endif
#ifdef __riscv_flen
DEF_UB(u_fmv_fx,   F1,  "fmv.x.w t0,ft0\n fmv.w.x ft0,t0")
#endif
#ifdef __riscv_flen
DEF_UB(u_flw_use,  F1 "fsw ft1,0(a5)\n", "flw ft0,0(a5)\n fadd.s ft2,ft0,ft0")
#endif
#ifdef __riscv_flen
DEF_UB(u_fsw,      F1,  "fsw ft0,0(a5)")
#endif
#ifdef __riscv_flen
DEF_UB(u_int_fp,   F1 ONE, "add t2,t2,t1\n fadd.s ft2,ft0,ft1")
#endif
DEF_UB(u_csr,      "",  "csrr t0, mcycle")
DEF_UB(u_fence,    "",  "fence")
DEF_UB(u_fencei,   "",  "fence.i")
DEF_UB(u_amo,      ONE, "amoadd.w t0,t1,(a5)")
DEF_UB(u_lrsc,     "",  "lr.w t0,(a5)\n sc.w t1,t0,(a5)")

typedef struct { const char *name; uint64_t (*f32)(void); uint64_t (*f64)(void); int k; const char *what; } ub_t;
#define U(n, k, what, desc) { desc, n##_32, n##_64, k, what }
static const ub_t ubs[] = {
  U(u_nop,       1, "thru", "nop"),
  U(u_add_dep,   1, "lat",  "add (dependent chain)"),
  U(u_add_ind,   4, "thru", "add (independent)"),
  U(u_c_add,     1, "lat",  "c.add (RVC, dependent)"),
  U(u_lw_ind,    4, "thru", "lw (independent)"),
  U(u_lw_use,    1, "pair", "lw + use (pair)"),
  U(u_lw_chase,  1, "lat",  "lw pointer-chase (load-to-use)"),
  U(u_sw,        4, "thru", "sw"),
  U(u_st_ld,     1, "pair", "sw -> lw same addr (pair)"),
  U(u_br_nt,     1, "thru", "branch not-taken"),
  U(u_br_t,      1, "thru", "branch taken"),
  U(u_br_t_mis,  1, "thru", "branch taken, target pc[1]=1"),
  U(u_jal,       1, "thru", "jal"),
  U(u_jalr,      1, "pair", "auipc + jalr (pair)"),
  U(u_mul_dep,   1, "lat",  "mul (dependent)"),
  U(u_mul_ind,   4, "thru", "mul (independent)"),
  U(u_mulh_dep,  1, "lat",  "mulh (dependent)"),
  U(u_div_dep,   1, "lat",  "div (dependent)"),
  U(u_div_ind,   2, "thru", "div (independent)"),
  U(u_rem_dep,   1, "lat",  "remu (dependent)"),
#ifdef __riscv_flen
  U(u_fadd_dep,  1, "lat",  "fadd.s (dependent)"),
#endif
#ifdef __riscv_flen
  U(u_fadd_ind,  4, "thru", "fadd.s (independent)"),
#endif
#ifdef __riscv_flen
  U(u_fmul_dep,  1, "lat",  "fmul.s (dependent)"),
#endif
#ifdef __riscv_flen
  U(u_fmul_ind,  4, "thru", "fmul.s (independent)"),
#endif
#ifdef __riscv_flen
  U(u_fmadd_dep, 1, "lat",  "fmadd.s (dependent)"),
#endif
#ifdef __riscv_flen
  U(u_fdiv_dep,  1, "lat",  "fdiv.s (dependent)"),
#endif
#ifdef __riscv_flen
  U(u_fsqrt_dep, 1, "lat",  "fsqrt.s (dependent)"),
#endif
#ifdef __riscv_flen
  U(u_fcmp,      1, "thru", "feq.s"),
#endif
#ifdef __riscv_flen
  U(u_fminmax,   1, "lat",  "fmin.s (dependent)"),
#endif
#ifdef __riscv_flen
  U(u_fsgnj,     1, "lat",  "fsgnj.s (dependent)"),
#endif
#ifdef __riscv_flen
  U(u_fcvt_sw,   1, "thru", "fcvt.s.w"),
#endif
#ifdef __riscv_flen
  U(u_fcvt_ws,   1, "thru", "fcvt.w.s"),
#endif
#ifdef __riscv_flen
  U(u_fmv_fx,    1, "pair", "fmv.x.w <-> fmv.w.x (pair)"),
#endif
#ifdef __riscv_flen
  U(u_flw_use,   1, "pair", "flw + fadd use (pair)"),
#endif
#ifdef __riscv_flen
  U(u_fsw,       1, "thru", "fsw"),
#endif
#ifdef __riscv_flen
  U(u_int_fp,    1, "pair", "add + fadd.s interleaved (pair)"),
#endif
  U(u_csr,       1, "thru", "csrr mcycle"),
  U(u_fence,     1, "thru", "fence"),
  U(u_fencei,    1, "thru", "fence.i"),
  U(u_amo,       1, "thru", "amoadd.w"),
  U(u_lrsc,      1, "pair", "lr.w + sc.w (pair)"),
};

static void micro(void) {
  printf("\n[1] INSTRUCTION LATENCY / THROUGHPUT  (cycles per instruction, loop overhead removed)\n");
  printf("  %-34s %-5s %10s\n", "instruction", "kind", "cycles");
  for (unsigned i = 0; i < sizeof ubs / sizeof ubs[0]; i++) {
    uint64_t a = ubs[i].f32(), b = ubs[i].f64();
    uint64_t d = b > a ? b - a : 0;
    printf("  %-34s %-5s %6s", ubs[i].name, ubs[i].what, "");
    print_fix(d, 32ull * ubs[i].k * LOOPS, 2);
    putchar('\n');
  }
}

/* ------------------------------------------------------------------ */
/* 2. memory system                                                    */
/* ------------------------------------------------------------------ */
#ifndef MEM_WORDS
#define MEM_WORDS 8192              /* 32 KB working buffers */
#endif
static uint32_t bufA[MEM_WORDS] __attribute__((aligned(64)));
static uint32_t bufB[MEM_WORDS] __attribute__((aligned(64)));

static uint32_t lcg_state = 12345;
static uint32_t lcg(void) { lcg_state = lcg_state * 1664525u + 1013904223u; return lcg_state; }

static void report_bw(const char *name, uint64_t bytes, uint64_t cyc) {
  printf("  %-34s ", name);
  print_fix(bytes, cyc, 3); printf(" B/cycle   = ");
  print_fix(bytes * (CPU_HZ / 1000u), cyc * 1000ull, 1); printf(" MB/s\n");
}

static void memory(void) {
  printf("\n[2] MEMORY SYSTEM  (buffer = %u KB)\n", (unsigned)(MEM_WORDS * 4 / 1024));
  /* random cyclic permutation for pointer chasing (defeats any prefetch) */
  for (uint32_t i = 0; i < MEM_WORDS; i++) bufB[i] = i;
  for (uint32_t i = MEM_WORDS - 1; i > 0; i--) { uint32_t j = lcg() % i; uint32_t t = bufB[i]; bufB[i] = bufB[j]; bufB[j] = t; }
  for (uint32_t i = 0; i < MEM_WORDS; i++) bufA[bufB[i]] = (uint32_t)&bufA[bufB[(i + 1) % MEM_WORDS]];

  uint32_t *p = (uint32_t *)bufA[0];
  uint64_t c = rdcycle64();
  for (uint32_t i = 0; i < MEM_WORDS; i++) p = (uint32_t *)*p;
  c = rdcycle64() - c;
  printf("  %-34s ", "random pointer chase (incl. loop)"); print_fix(c, MEM_WORDS, 2);
  printf(" cycles/load = "); print_fix(c * 1000000000ull / MEM_WORDS, CPU_HZ, 1); printf(" ns  [%x]\n", (uint32_t)p & 0xf);

  /* sequential read */
  uint32_t s = 0;
  c = rdcycle64();
  for (uint32_t i = 0; i < MEM_WORDS; i += 8)
    s += bufA[i] ^ bufA[i+1] ^ bufA[i+2] ^ bufA[i+3] ^ bufA[i+4] ^ bufA[i+5] ^ bufA[i+6] ^ bufA[i+7];
  c = rdcycle64() - c;
  report_bw("sequential read (lw x8 unrolled)", MEM_WORDS * 4ull, c);

  c = rdcycle64();
  for (uint32_t i = 0; i < MEM_WORDS; i += 8) {
    bufB[i] = s; bufB[i+1] = s; bufB[i+2] = s; bufB[i+3] = s;
    bufB[i+4] = s; bufB[i+5] = s; bufB[i+6] = s; bufB[i+7] = s;
  }
  c = rdcycle64() - c;
  report_bw("sequential write (sw x8 unrolled)", MEM_WORDS * 4ull, c);

  c = rdcycle64();
  memcpy(bufB, bufA, sizeof bufA);
  c = rdcycle64() - c;
  report_bw("memcpy (word loop)", MEM_WORDS * 4ull, c);

  c = rdcycle64();
  volatile uint8_t *b8 = (volatile uint8_t *)bufB;
  for (uint32_t i = 0; i < 4096; i++) b8[i] = (uint8_t)i;
  c = rdcycle64() - c;
  report_bw("byte stores (sb)", 4096, c);
}

/* ------------------------------------------------------------------ */
/* 3. trap / interrupt latency                                        */
/* ------------------------------------------------------------------ */
static void latency(void) {
  printf("\n[3] TRAP / INTERRUPT LATENCY  (cycles; handler samples mcycle at its 3rd instruction)\n");
  uint32_t best_e = ~0u, best_rt = ~0u, best_i = ~0u, sum_i = 0;
  for (int r = 0; r < 8; r++) {
    uint32_t c0, c1;
    asm volatile("csrr %0, mcycle\n ecall\n csrr %1, mcycle" : "=&r"(c0), "=&r"(c1) :: "memory");
    if (trap_cycle - c0 < best_e) best_e = trap_cycle - c0;
    if (c1 - c0 < best_rt) best_rt = c1 - c0;
  }
  printf("  %-34s %u\n", "ecall -> handler entry", best_e);
  printf("  %-34s %u\n", "ecall round trip (C handler+mret)", best_rt);

  set_csr(mie, 1u << 3);           /* MSIE */
  set_csr(mstatus, 1u << 3);       /* MIE  */
  int n = 16;
  for (int r = 0; r < n; r++) {
    uint32_t before = irq_msip_count, c0;
    asm volatile("li t0, 1\n csrr %0, mcycle\n sw t0, 0(%1)\n"
                 : "=&r"(c0) : "r"(&CLINT_MSIP) : "t0", "memory");
    while (irq_msip_count == before) ;
    uint32_t l = trap_cycle - c0; sum_i += l; if (l < best_i) best_i = l;
  }
  clear_csr(mstatus, 1u << 3);
  printf("  %-34s min %u  avg ", "SW interrupt (msip store->handler)", best_i);
  print_fix(sum_i, n, 1); putchar('\n');

  /* timer interrupt: delivered at all, and how late relative to mtimecmp */
  set_csr(mie, 1u << 7);
  set_csr(mstatus, 1u << 3);
  uint32_t before = irq_mtip_count;
  timer_period = 0;
  timer_arm(20);
  uint64_t t0 = rdcycle64();
  while (irq_mtip_count == before && rdcycle64() - t0 < 2000000) ;
  clear_csr(mstatus, 1u << 3);
  clear_csr(mie, (1u << 7) | (1u << 3));
  CHECK(irq_mtip_count != before, "timer interrupt never arrived\n");
  printf("  %-34s %s\n", "machine timer interrupt", irq_mtip_count != before ? "delivered" : "MISSING");
}

/* ------------------------------------------------------------------ */
/* 4. application kernels                                             */
/* ------------------------------------------------------------------ */
#define N 16
static int32_t IA[N][N], IB[N][N], IC[N][N];
static float FA[N][N], FB[N][N], FC[N][N];

static __attribute__((noinline)) uint32_t k_int_matmul(void) {
  for (int i = 0; i < N; i++) for (int j = 0; j < N; j++) { IA[i][j] = i * 7 - j; IB[i][j] = j * 3 + i; }
  for (int i = 0; i < N; i++) for (int j = 0; j < N; j++) {
    int32_t s = 0; for (int k = 0; k < N; k++) s += IA[i][k] * IB[k][j]; IC[i][j] = s; }
  uint32_t h = 0; for (int i = 0; i < N; i++) for (int j = 0; j < N; j++) h = h * 31 + IC[i][j];
  return h;
}
static __attribute__((noinline)) uint32_t k_sort(void) {
  static uint32_t a[1024];
  lcg_state = 7; for (int i = 0; i < 1024; i++) a[i] = lcg();
  /* shell sort: branchy, load/store heavy */
  for (int gap = 512; gap > 0; gap >>= 1)
    for (int i = gap; i < 1024; i++) { uint32_t t = a[i]; int j = i;
      for (; j >= gap && a[j - gap] > t; j -= gap) a[j] = a[j - gap]; a[j] = t; }
  uint32_t h = 0; for (int i = 0; i < 1024; i++) h = h * 33 ^ a[i];
  for (int i = 1; i < 1024; i++) CHECK(a[i-1] <= a[i], "sort order\n");
  return h;
}
static __attribute__((noinline)) uint32_t k_crc32(void) {
  uint32_t crc = 0xFFFFFFFFu; const uint8_t *p = (const uint8_t *)bufA;
  for (int i = 0; i < 8192; i++) { crc ^= p[i]; for (int b = 0; b < 8; b++) crc = (crc >> 1) ^ (0xEDB88320u & -(crc & 1)); }
  return ~crc;
}
static __attribute__((noinline)) uint32_t k_fp_matmul(void) {
  for (int i = 0; i < N; i++) for (int j = 0; j < N; j++) { FA[i][j] = (float)(i*3+j)/7.0f - 1.5f; FB[i][j] = (float)(i-j)*0.25f + 0.125f; }
  for (int i = 0; i < N; i++) for (int j = 0; j < N; j++) {
    float s = 0; for (int k = 0; k < N; k++) s += FA[i][k] * FB[k][j]; FC[i][j] = s; }
  uint32_t h = 0; for (int i = 0; i < N; i++) for (int j = 0; j < N; j++) { union { float f; uint32_t u; } x = { FC[i][j] }; h = h * 31 + x.u; }
  return h;
}

typedef struct { const char *name; uint32_t (*f)(void); uint32_t flops; } kern_t;
static const kern_t kerns[] = {
  { "int matmul 16x16",   k_int_matmul, 0 },
  { "shell sort 1024",    k_sort,       0 },
  { "crc32 8KB (bitwise)", k_crc32,     0 },
  { "fp32 matmul 16x16",  k_fp_matmul,  2 * N * N * N },
};

static void apps(void) {
  printf("\n[4] APPLICATION KERNELS  (@ %u MHz)\n", (unsigned)(CPU_HZ / 1000000u));
  printf("  %-22s %10s %10s %7s %7s %9s %9s\n", "kernel", "cycles", "instret", "CPI", "IPC", "MIPS", "MFLOPS");
  uint64_t tc = 0, ti = 0;
  for (unsigned i = 0; i < sizeof kerns / sizeof kerns[0]; i++) {
    kerns[i].f();                                   /* warm-up / init */
    uint64_t c = rdcycle64(), n = rdinstret64();
    uint32_t h = kerns[i].f();
    n = rdinstret64() - n; c = rdcycle64() - c;
    tc += c; ti += n;
    printf("  %-22s %10u %10u ", kerns[i].name, (uint32_t)c, (uint32_t)n);
    print_fix(c, n, 3); printf("  "); print_fix(n, c, 3); printf("  ");
    print_fix(n * (CPU_HZ / 1000u), c * 1000ull, 1); printf("   ");
    if (kerns[i].flops) print_fix((uint64_t)kerns[i].flops * (CPU_HZ / 1000u), c * 1000ull, 2); else puts_raw("    -");
    printf("   [%08x]\n", h);
  }
  printf("  %-22s %10u %10u ", "TOTAL", (uint32_t)tc, (uint32_t)ti);
  print_fix(tc, ti, 3); printf("  "); print_fix(ti, tc, 3); printf("  ");
  print_fix(ti * (CPU_HZ / 1000u), tc * 1000ull, 1); printf("\n");
}

int main(void) {
  printf("\n==================================================\n");
  printf(" ALPANA_SOC perf suite | Shakti E-class %s\n", ISA_STR);
  printf(" CPU_HZ = %u%s\n", (unsigned)CPU_HZ,
#ifdef SIM
         "  (Verilator RTL sim: MHz-based figures assume this clock)"
#else
         ""
#endif
  );
  printf("==================================================\n");
  uint64_t c0 = rdcycle64(), i0 = rdinstret64();
#ifndef ONLY
  micro();
  memory();
  latency();
  apps();
#else
  ONLY();
#endif
  uint64_t c = rdcycle64() - c0, n = rdinstret64() - i0;
  printf("\n whole suite: %u cycles, %u instr, CPI ", (uint32_t)c, (uint32_t)n); print_fix(c, n, 3);
  printf("\n RESULT: %s (%d failures)\n", g_fail ? "FAIL" : "PASS", g_fail);
  halt(g_fail);
  return 0;
}
