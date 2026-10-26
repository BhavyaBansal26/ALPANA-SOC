/*
 * ALPANA_SOC "hardcore" self-checking stress test
 *   A. random ISA kernels (ALU, M-ext corner cases, FPU incl. NaN/inf/subnormal, control flow)
 *      checked against golden checksums generated on the host with exact RISC-V semantics
 *   B. memory: March C- (word), byte/halfword lanes, address-in-address over free RAM
 *   C. atomics: AMO*, LR/SC
 *   D. interrupt storm: kernels re-run under a high-rate timer + software interrupt load
 *   E. soak: A + D repeated STRESS_ROUNDS times (0 = forever, heartbeat every round)
 */
#include "lib.h"
#include "stress_kern.h"
#include "stress_list.h"
#include "golden.h"

#ifndef MEMTEST_BYTES
#define MEMTEST_BYTES (64 * 1024)
#endif
#ifndef STRESS_ROUNDS
#define STRESS_ROUNDS 3
#endif
#ifndef STORM_PERIOD
#define STORM_PERIOD 40           /* mtime ticks between timer interrupts */
#endif
extern uint32_t _heap_start[];

static int run_kernels(int verbose) {
  int bad = 0;
#define RUN(name, call) {                                                        \
    uint64_t c = rdcycle64(); uint32_t got = call; c = rdcycle64() - c;          \
    int ok = got == GOLD_##name; bad += !ok;                                     \
    if (verbose || !ok) printf("  %-10s %08x  expect %08x  %s  (%u cycles)\n",   \
                               #name, got, GOLD_##name, ok ? "ok" : "MISMATCH", (uint32_t)c); }
  STRESS_TESTS(RUN)
  return bad;
}

static int march(volatile uint32_t *m, uint32_t words) {
  int bad = 0;
  const uint32_t pat[2] = { 0x00000000u, 0xFFFFFFFFu };
  for (int p = 0; p < 2; p++) {
    uint32_t z = pat[p], o = ~z;
    for (uint32_t i = 0; i < words; i++) m[i] = z;                                   /* up w0      */
    for (uint32_t i = 0; i < words; i++) { bad += m[i] != z; m[i] = o; }              /* up r0,w1   */
    for (uint32_t i = 0; i < words; i++) { bad += m[i] != o; m[i] = z; }              /* up r1,w0   */
    for (uint32_t i = words; i-- > 0;)   { bad += m[i] != z; m[i] = o; }              /* down r0,w1 */
    for (uint32_t i = words; i-- > 0;)   { bad += m[i] != o; m[i] = z; }              /* down r1,w0 */
    for (uint32_t i = 0; i < words; i++)   bad += m[i] != z;                          /* r0         */
  }
  for (uint32_t i = 0; i < words; i++) m[i] = (uint32_t)&m[i] ^ 0xA5A5A5A5u;        /* addr-in-addr */
  for (uint32_t i = 0; i < words; i++) bad += m[i] != ((uint32_t)&m[i] ^ 0xA5A5A5A5u);
  for (uint32_t i = 0; i < words; i++) m[i] = ~((uint32_t)&m[i]);
  for (uint32_t i = 0; i < words; i++) bad += m[i] != ~((uint32_t)&m[i]);
  /* byte / halfword lanes */
  volatile uint8_t *b = (volatile uint8_t *)m; volatile uint16_t *hw = (volatile uint16_t *)m;
  for (uint32_t i = 0; i < words * 4; i++) b[i] = (uint8_t)(i * 7 + 3);
  for (uint32_t i = 0; i < words; i++) {
    uint32_t e = 0; for (int k = 3; k >= 0; k--) e = e << 8 | (uint8_t)((i * 4 + k) * 7 + 3);
    bad += m[i] != e;
  }
  for (uint32_t i = 0; i < words * 2; i++) hw[i] = (uint16_t)(i * 0x9E37u);
  for (uint32_t i = 0; i < words * 2; i++) bad += hw[i] != (uint16_t)(i * 0x9E37u);
  for (uint32_t i = 0; i < words * 4; i++) { int8_t v = (int8_t)b[i]; int32_t sv = *(volatile int8_t *)&b[i]; bad += sv != v; }
  return bad;
}

static int atomics(void) {
  static volatile uint32_t v[4];
  int bad = 0; uint32_t old;
  v[0] = 10;
  asm volatile("amoadd.w %0, %2, (%1)" : "=r"(old) : "r"(&v[0]), "r"(5) : "memory");   bad += old != 10 || v[0] != 15;
  asm volatile("amoswap.w %0, %2, (%1)" : "=r"(old) : "r"(&v[0]), "r"(77) : "memory"); bad += old != 15 || v[0] != 77;
  asm volatile("amoand.w %0, %2, (%1)" : "=r"(old) : "r"(&v[0]), "r"(0x0f) : "memory"); bad += old != 77 || v[0] != (77 & 15);
  asm volatile("amoor.w %0, %2, (%1)" : "=r"(old) : "r"(&v[0]), "r"(0xf0) : "memory");  bad += v[0] != ((77 & 15) | 0xf0);
  asm volatile("amoxor.w %0, %2, (%1)" : "=r"(old) : "r"(&v[0]), "r"(0xff) : "memory"); bad += v[0] != (((77 & 15) | 0xf0) ^ 0xff);
  v[1] = (uint32_t)-5;
  asm volatile("amomin.w %0, %2, (%1)" : "=r"(old) : "r"(&v[1]), "r"(3) : "memory");   bad += v[1] != (uint32_t)-5;
  asm volatile("amomaxu.w %0, %2, (%1)" : "=r"(old) : "r"(&v[1]), "r"(3) : "memory");  bad += v[1] != (uint32_t)-5;
  asm volatile("amomax.w %0, %2, (%1)" : "=r"(old) : "r"(&v[1]), "r"(3) : "memory");   bad += v[1] != 3;
  asm volatile("amominu.w %0, %2, (%1)" : "=r"(old) : "r"(&v[1]), "r"(1) : "memory");  bad += v[1] != 1;
  /* LR/SC counter increment loop */
  v[2] = 0;
  for (int i = 0; i < 100; i++) {
    uint32_t t, fail;
    asm volatile("1: lr.w %0, (%2)\n addi %0, %0, 1\n sc.w %1, %0, (%2)\n bnez %1, 1b"
                 : "=&r"(t), "=&r"(fail) : "r"(&v[2]) : "memory");
  }
  bad += v[2] != 100;
  /* SC without reservation must fail (after an intervening SC) */
  uint32_t r1, r2, t;
  asm volatile("lr.w %0, (%3)\n sc.w %1, %0, (%3)\n sc.w %2, %0, (%3)" : "=&r"(t), "=&r"(r1), "=&r"(r2) : "r"(&v[3]) : "memory");
  bad += r1 != 0 || r2 == 0;
  return bad;
}

static int storm(void) {
  uint32_t t0 = irq_mtip_count, s0 = irq_msip_count;
  timer_period = STORM_PERIOD;
  set_csr(mie, (1u << 7) | (1u << 3));
  timer_arm(STORM_PERIOD);
  set_csr(mstatus, 1u << 3);
  int bad = 0;
  for (int k = 0; k < 2; k++) {
    CLINT_MSIP = 1;                      /* software interrupt on top of the timer load */
    bad += run_kernels(0);
  }
  clear_csr(mstatus, 1u << 3);
  timer_period = 0; timer_disarm();
  clear_csr(mie, (1u << 7) | (1u << 3));
  uint32_t nt = irq_mtip_count - t0, ns = irq_msip_count - s0;
  printf("  kernels x2 under interrupt load: %u timer + %u sw interrupts taken, %d mismatches\n", nt, ns, bad);
  if (nt < 10) { printf("  FAIL: too few timer interrupts\n"); bad++; }
  if (ns < 1)  { printf("  FAIL: sw interrupt not taken\n"); bad++; }
  return bad;
}

static int csr_checks(void) {
  int bad = 0;
  write_csr(mscratch, 0x5a5aa5a5u); bad += read_csr(mscratch) != 0x5a5aa5a5u;
  uint32_t i0 = read_csr(minstret); asm volatile("nop\n nop\n nop\n nop"); uint32_t i1 = read_csr(minstret);
  bad += (i1 - i0) < 4 || (i1 - i0) > 8;
  uint32_t misa = read_csr(misa);
#ifdef __riscv_flen
  const char *ext = "IMAFC";
#else
  const char *ext = "IMAC";
#endif
  for (const char *e = ext; *e; e++) bad += !(misa & (1u << (*e - 'A')));
  printf("  misa=%08x  minstret delta(4 nops)=%u  mscratch ok=%d\n", misa, i1 - i0, read_csr(mscratch) == 0x5a5aa5a5u);
  return bad;
}

int main(void) {
  printf("\n==================================================\n");
  printf(" ALPANA_SOC hardcore stress test\n");
  printf("==================================================\n");
  int total = 0, b;

  printf("\n[A] random ISA kernels vs host golden\n");
  b = run_kernels(1); total += b;

  printf("\n[B] memory test, %u KB at %08x\n", (unsigned)(MEMTEST_BYTES / 1024), (uint32_t)_heap_start);
  uint64_t c = rdcycle64();
  b = march((volatile uint32_t *)_heap_start, MEMTEST_BYTES / 4); total += b;
  printf("  march C- / addr-in-addr / byte+half lanes: %d errors  (%u Mcycles)\n", b, (uint32_t)((rdcycle64() - c) / 1000000));

  printf("\n[C] atomics + CSRs\n");
  b = atomics(); total += b; printf("  AMO/LR/SC: %d errors\n", b);
  b = csr_checks(); total += b; printf("  CSR: %d errors\n", b);

  printf("\n[D] interrupt storm\n");
  total += storm();

  printf("\n[E] soak (%d rounds%s)\n", STRESS_ROUNDS, STRESS_ROUNDS ? "" : " = forever");
  for (int r = 1; STRESS_ROUNDS == 0 || r <= STRESS_ROUNDS; r++) {
    b = run_kernels(0) + storm();
    b += march((volatile uint32_t *)_heap_start, MEMTEST_BYTES / 4);
    total += b;
    printf("  round %d: %s  (total errors %d)\n", r, b ? "ERRORS" : "clean", total);
  }

  printf("\n RESULT: %s (%d errors)\n", total ? "FAIL" : "PASS", total);
  halt(total);
  return 0;
}
