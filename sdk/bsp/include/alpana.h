/***************************************************************************
 * Project           : ALPANA SDK
 * Name of the file  : alpana.h
 * Brief             : ALPANA_SOC board support for the Digilent Nexys 4 DDR:
 *                     on-board LEDs / switches / buttons, Pmod JD GPIOs,
 *                     pin-mux (Pmod JA/JB), cycle counter and delays.
 *                     Register map taken from the ALPANA_SOC RTL
 *                     (mkgpio_e32.v, mkpinmux_e32.v, mkmixed_cluster_e32.v).
 ***************************************************************************/
#ifndef ALPANA_H
#define ALPANA_H
#include <stdint.h>
#include "platform.h"

/* ------------------------------------------------------------- fast on-chip RAM
 * The core has no cache: every instruction fetched from DDR2 costs ~48 cycles
 * (CPI ~55). Mark hot functions ALPANA_FAST: they are linked to run from the
 * free part of the on-chip boot RAM (0x1C80-0x2BFF, 3.9 KB) and copied there
 * by init() before main(). ALPANA_FAST_BSS puts zero-initialised data
 * (buffers, tables) in boot RAM 0x2C10-0x2FBF (~0.9 KB).
 * The linker stops with an error if either area overflows.
 * Build with FLAGS="-D ALPANA_FASTRAM=0" to keep everything in DDR2.
 */
#ifndef ALPANA_FASTRAM
#define ALPANA_FASTRAM 1
#endif
#if ALPANA_FASTRAM
#define ALPANA_FAST     __attribute__((section(".fasttext"), noinline))
#define ALPANA_FAST_BSS __attribute__((section(".fastbss")))
#else
#define ALPANA_FAST
#define ALPANA_FAST_BSS
#endif

/* UART0 registers as volatile pointers (uart_struct fields are not volatile) */
#define ALPANA_UART0_TX     (*(volatile uint32_t *)(0x00011300 + 0x04))
#define ALPANA_UART0_STATUS (*(volatile uint8_t  *)(0x00011300 + 0x0C))
#define ALPANA_UART_TX_FULL  0x02
#define ALPANA_UART_TX_EMPTY 0x01
/* raw byte to UART0, inline (use in ALPANA_FAST loops) */
static inline __attribute__((always_inline)) void alpana_uart_putb_fast(uint8_t c) {
  while (ALPANA_UART0_STATUS & ALPANA_UART_TX_FULL) ;
  ALPANA_UART0_TX = c;
}

/* ------------------------------------------------------------- GPIO */
#define ALPANA_GPIO_DIR   (*(volatile uint32_t *)(GPIO_START + 0x00))  /* 1 = output */
#define ALPANA_GPIO_DATA  (*(volatile uint32_t *)(GPIO_START + 0x08))  /* read = pins, write = outputs */

/* Nexys 4 DDR on-board I/O wired to dedicated GPIOs (see constraints.xdc) */
#define LED0   GPIO16
#define LED1   GPIO17
#define LED2   GPIO18
#define LED3   GPIO19
#define LED_ALL (LED0 | LED1 | LED2 | LED3)
#define SW0    GPIO4
#define SW1    GPIO7
#define SW2    GPIO8
#define SW3    GPIO14
#define SW4    GPIO15
#define BTN_C  GPIO20
#define BTN_U  GPIO21
#define BTN_L  GPIO22
#define BTN_R  GPIO23
/* Pmod JD: 8 general-purpose 3.3 V GPIOs */
#define JD1    GPIO24
#define JD2    GPIO25
#define JD3    GPIO26
#define JD4    GPIO27
#define JD7    GPIO28
#define JD8    GPIO29
#define JD9    GPIO30
#define JD10   GPIO31

void     alpana_gpio_output(uint32_t mask);          /* make pins outputs */
void     alpana_gpio_input(uint32_t mask);           /* make pins inputs  */
void     alpana_gpio_set(uint32_t mask);             /* drive high */
void     alpana_gpio_clear(uint32_t mask);           /* drive low  */
void     alpana_gpio_toggle(uint32_t mask);
uint32_t alpana_gpio_read(void);                     /* all 32 pin levels */
static inline int alpana_gpio_get(uint32_t mask) { return (alpana_gpio_read() & mask) != 0; }

/* ------------------------------------------------------------- pin mux
 * One 32-bit register, 2 bits per I/O cell. Pmod pin -> cell -> functions:
 *   JA1  io7   0:GPIO  1:UART1_RX
 *   JA2  io8   0:GPIO  1:UART1_TX
 *   JA3  io9   0:GPIO  1:UART2_RX
 *   JA4  io10  0:GPIO  1:UART2_TX  2:PWM0
 *   JA7  io12  0:GPIO              2:PWM1
 *   JA8  io13  0:GPIO              2:PWM2
 *   JA9  io16  0:GPIO              2:PWM3
 *   JA10 io17  0:GPIO  1:SPI1_NSS  2:PWM4
 *   JB1  io18  0:GPIO  1:SPI1_MOSI 2:PWM5
 *   JB2  io19  0:GPIO  1:SPI1_MISO
 *   JB3  io20  0:GPIO  1:SPI1_SCLK
 */
#define ALPANA_PINMUX (*(volatile uint32_t *)PINMUX_CONFIGURE_REG)
enum { PIN_JA1 = 0, PIN_JA2, PIN_JA3, PIN_JA4, PIN_JA7, PIN_JA8, PIN_JA9, PIN_JA10, PIN_JB1, PIN_JB2, PIN_JB3 };
#define PINMUX_GPIO 0
#define PINMUX_ALT1 1          /* UART / SPI1 */
#define PINMUX_ALT2 2          /* PWM         */
void alpana_pinmux_set(int pin, int function);       /* pin = PIN_JAx / PIN_JBx */
#define PINMUX_SPI1_ALL ((1u << 14) | (1u << 16) | (1u << 18) | (1u << 20))  /* JA10, JB1..JB3 -> SPI1 */

/* ------------------------------------------------------------- time */
static inline uint64_t alpana_cycles(void) {           /* 64-bit mcycle */
  uint32_t hi, lo, hi2;
  do {
    asm volatile ("csrr %0, mcycleh" : "=r"(hi));
    asm volatile ("csrr %0, mcycle"  : "=r"(lo));
    asm volatile ("csrr %0, mcycleh" : "=r"(hi2));
  } while (hi != hi2);
  return ((uint64_t)hi << 32) | lo;
}
static inline uint64_t alpana_instret(void) {
  uint32_t hi, lo, hi2;
  do {
    asm volatile ("csrr %0, minstreth" : "=r"(hi));
    asm volatile ("csrr %0, minstret"  : "=r"(lo));
    asm volatile ("csrr %0, minstreth" : "=r"(hi2));
  } while (hi != hi2);
  return ((uint64_t)hi << 32) | lo;
}
#define ALPANA_CYCLES_PER_US (CLOCK_FREQUENCY / 1000000)
void alpana_delay_us(uint32_t us);
void alpana_delay_ms(uint32_t ms);
uint32_t alpana_millis(void);                       /* ms since reset (from mcycle) */

/* prints "ALPANA_SOC <clock> MHz, <mem> ..." banner, returns misa */
uint32_t alpana_banner(void);

/* UART0 raw byte (no '\n' -> "\r\n" translation): for binary streams */
void alpana_uart_putb(uint8_t c);
void alpana_uart_set_baud(uint32_t baud);           /* e.g. 115200, 1000000, 2500000 */

#endif
