/* ALPANA_SOC bare-metal benchmark library (Shakti E-class, RV32IMAFC) */
#ifndef ALPANA_LIB_H
#define ALPANA_LIB_H
#include <stdint.h>
#include <stddef.h>

#ifndef CPU_HZ
#define CPU_HZ 80000000u          /* core clock: clk_out1 of clk_wiz_0 */
#endif
#ifndef BAUD_DIV
#define BAUD_DIV 43               /* CPU_HZ / (16 * 115200) */
#endif

#define UART_BASE   0x00011300u
#define CLINT_BASE  0x02000000u
#define CLINT_MSIP     (*(volatile uint32_t *)(CLINT_BASE + 0x0000))
#define CLINT_MTIMECMP ((volatile uint32_t *)(CLINT_BASE + 0x4000))
#define CLINT_MTIME    ((volatile uint32_t *)(CLINT_BASE + 0xBFF8))

#define read_csr(r) ({ uint32_t __v; asm volatile ("csrr %0, " #r : "=r"(__v)); __v; })
#define write_csr(r, v) asm volatile ("csrw " #r ", %0" :: "rK"(v))
#define set_csr(r, v)   asm volatile ("csrs " #r ", %0" :: "rK"(v))
#define clear_csr(r, v) asm volatile ("csrc " #r ", %0" :: "rK"(v))

static inline uint64_t rdcycle64(void) {
  uint32_t hi, lo, hi2;
  do { hi = read_csr(mcycleh); lo = read_csr(mcycle); hi2 = read_csr(mcycleh); } while (hi != hi2);
  return ((uint64_t)hi << 32) | lo;
}
static inline uint64_t rdinstret64(void) {
  uint32_t hi, lo, hi2;
  do { hi = read_csr(minstreth); lo = read_csr(minstret); hi2 = read_csr(minstreth); } while (hi != hi2);
  return ((uint64_t)hi << 32) | lo;
}
static inline uint64_t rdmtime(void) {
  uint32_t hi, lo, hi2;
  do { hi = CLINT_MTIME[1]; lo = CLINT_MTIME[0]; hi2 = CLINT_MTIME[1]; } while (hi != hi2);
  return ((uint64_t)hi << 32) | lo;
}

void uart_init(void);
int  putchar(int c);
void puts_raw(const char *s);
int  printf(const char *fmt, ...);
void print_fix(uint64_t num, uint64_t den, int decimals);  /* prints num/den */
void print_u64(uint64_t v);
void halt(int code);   /* end of program: sim -> signature dump, board -> spin */

/* trap support (crt.S + lib.c) */
extern volatile uint32_t trap_cycle;   /* mcycle captured on trap entry   */
extern volatile uint32_t irq_msip_count, irq_mtip_count, ecall_count;
extern volatile uint32_t timer_period; /* 0 = one-shot */
void timer_arm(uint32_t delta);
void timer_disarm(void);

void *memcpy(void *d, const void *s, size_t n);
void *memset(void *d, int c, size_t n);
int   strcmp(const char *a, const char *b);
char *strcpy(char *d, const char *s);
size_t strlen(const char *s);

extern int g_fail;   /* global failure counter */
#define CHECK(cond, ...) do { if (!(cond)) { g_fail++; printf("  FAIL: " __VA_ARGS__); } } while (0)
#endif
