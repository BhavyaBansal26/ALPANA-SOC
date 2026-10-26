#include "lib.h"
#include <stdarg.h>

int g_fail;
volatile uint32_t trap_cycle;
volatile uint32_t irq_msip_count, irq_mtip_count, ecall_count;
volatile uint32_t timer_period;

#define UART_REG8(off) (*(volatile uint8_t  *)(UART_BASE + (off)))
#define UART_REG16(off) (*(volatile uint16_t *)(UART_BASE + (off)))

void uart_init(void) {
#ifndef SIM
  UART_REG16(0x0) = BAUD_DIV;          /* sim testbench UART is fixed at divisor 5 */
#endif
}

int putchar(int c) {
  if (c == '\n') putchar('\r');
#ifdef SIM
  while (!(UART_REG8(0xC) & 0x2)) ;    /* sim UART: bit1 = tx fifo not full */
#else
  while (UART_REG8(0xC) & 0x2) ;       /* ALPANA board UART (uart_v2): bit1 = tx FULL */
#endif
  UART_REG8(0x4) = (uint8_t)c;
  return c;
}
void puts_raw(const char *s) { while (*s) putchar(*s++); }

static void put_u64(uint64_t v, int base, int width, char pad) {
  char buf[24]; int n = 0;
  do { int d = (int)(v % base); buf[n++] = d < 10 ? '0' + d : 'a' + d - 10; v /= base; } while (v);
  while (n < width) buf[n++] = pad;
  while (n) putchar(buf[--n]);
}
void print_u64(uint64_t v) { put_u64(v, 10, 0, ' '); }

int printf(const char *fmt, ...) {
  va_list ap; va_start(ap, fmt);
  for (; *fmt; fmt++) {
    if (*fmt != '%') { putchar(*fmt); continue; }
    fmt++;
    char pad = ' '; int width = 0, lng = 0, left = 0;
    if (*fmt == '-') { left = 1; fmt++; }
    if (*fmt == '0') { pad = '0'; fmt++; }
    while (*fmt >= '0' && *fmt <= '9') width = width * 10 + (*fmt++ - '0');
    while (*fmt == 'l') { lng++; fmt++; }
    switch (*fmt) {
      case 'd': {
        int64_t v = lng >= 2 ? va_arg(ap, int64_t) : va_arg(ap, int32_t);
        if (v < 0) { putchar('-'); v = -v; if (width) width--; }
        put_u64((uint64_t)v, 10, width, pad); break; }
      case 'u': put_u64(lng >= 2 ? va_arg(ap, uint64_t) : va_arg(ap, uint32_t), 10, width, pad); break;
      case 'x': put_u64(lng >= 2 ? va_arg(ap, uint64_t) : va_arg(ap, uint32_t), 16, width, pad); break;
      case 'c': putchar(va_arg(ap, int)); break;
      case 's': {
        const char *s = va_arg(ap, const char *); int n = (int)strlen(s);
        if (!left) while (n++ < width) putchar(' ');
        puts_raw(s);
        if (left) while (n++ < width) putchar(' ');
        break; }
      case '%': putchar('%'); break;
      default: putchar('?');
    }
  }
  va_end(ap);
  return 0;
}

/* fixed-point print of num/den with rounding, e.g. CPI = cycles/instret */
void print_fix(uint64_t num, uint64_t den, int decimals) {
  uint64_t scale = 1; for (int i = 0; i < decimals; i++) scale *= 10;
  if (den == 0) { puts_raw("n/a"); return; }
  uint64_t v = (num * scale + den / 2) / den;
  print_u64(v / scale);
  if (decimals) { putchar('.'); put_u64(v % scale, 10, decimals, '0'); }
}

void timer_arm(uint32_t delta) {
  uint64_t t = rdmtime() + delta;
  CLINT_MTIMECMP[1] = 0xFFFFFFFFu;
  CLINT_MTIMECMP[0] = (uint32_t)t;
  CLINT_MTIMECMP[1] = (uint32_t)(t >> 32);
}
void timer_disarm(void) { CLINT_MTIMECMP[1] = 0xFFFFFFFFu; CLINT_MTIMECMP[0] = 0xFFFFFFFFu; }

/* called from crt.S trap_entry with all caller-saved regs preserved */
uint32_t c_trap(uint32_t mcause, uint32_t mepc) {
  if (mcause == 0x80000003u) { CLINT_MSIP = 0; irq_msip_count++; return mepc; }
  if (mcause == 0x80000007u) {
    irq_mtip_count++;
    if (timer_period) timer_arm(timer_period); else timer_disarm();
    return mepc;
  }
  if (mcause == 11) { ecall_count++; return mepc + 4; }
  printf("\n*** UNEXPECTED TRAP mcause=%08x mepc=%08x mtval=%08x\n", mcause, mepc, read_csr(mtval));
  halt(2);
  return mepc;
}

void halt(int code) {
  while (!(UART_REG8(0xC) & 0x1)) ;          /* wait tx fifo empty */
  for (volatile int i = 0; i < 4000; i++) ;  /* let last char shift out */
#ifdef SIM
  static volatile uint32_t sig[4];
  sig[0] = code; sig[1] = 0x600d;
  volatile uint32_t *s = (volatile uint32_t *)0x20000;
  s[0] = (uint32_t)&sig[0]; s[2] = (uint32_t)&sig[4]; s[3] = 0;
#endif
  (void)code;
#ifndef SIM
  /* Park in boot RAM: the debugger cannot halt the core while it keeps
     fetching from DDR2, but it can halt it in on-chip boot RAM. */
  asm volatile ("csrci mstatus, 8");
  write_csr(mie, 0);
  CLINT_MTIMECMP[1] = 0xFFFFFFFFu;
  volatile uint32_t *park = (volatile uint32_t *)0x2C00;
  park[0] = 0x0000006fu;                     /* j .  */
  asm volatile ("fence; fence.i" ::: "memory");
  ((void (*)(void))0x2C00)();
#endif
  for (;;) asm volatile ("nop");   /* busy-wait: Shakti cannot be halted by the debugger while in WFI */
}

void *memcpy(void *d, const void *s, size_t n) {
  uint8_t *dp = d; const uint8_t *sp = s;
  if ((((uintptr_t)dp | (uintptr_t)sp | n) & 3) == 0) {
    uint32_t *dw = d; const uint32_t *sw = s;
    for (n >>= 2; n; n--) *dw++ = *sw++;
    return d;
  }
  while (n--) *dp++ = *sp++;
  return d;
}
void *memset(void *d, int c, size_t n) { uint8_t *p = d; while (n--) *p++ = (uint8_t)c; return d; }
int strcmp(const char *a, const char *b) { while (*a && *a == *b) { a++; b++; } return (uint8_t)*a - (uint8_t)*b; }
char *strcpy(char *d, const char *s) { char *r = d; while ((*d++ = *s++)) ; return r; }
size_t strlen(const char *s) { size_t n = 0; while (s[n]) n++; return n; }
