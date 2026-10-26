/***************************************************************************
 * Project           : ALPANA SDK
 * Name of the file  : alpana_board.c
 * Brief             : GPIO, pin-mux, timing and UART helpers for ALPANA_SOC
 ***************************************************************************/
#include "alpana.h"
#include "uart.h"
#include "utils.h"

static uint32_t gpio_out_shadow;       /* last value written to the data register */
static uint32_t pinmux_shadow;         /* pin-mux register is write-only */

void alpana_gpio_output(uint32_t mask) { ALPANA_GPIO_DIR = ALPANA_GPIO_DIR | mask; }
void alpana_gpio_input(uint32_t mask)  { ALPANA_GPIO_DIR = ALPANA_GPIO_DIR & ~mask; }
void alpana_gpio_set(uint32_t mask)    { gpio_out_shadow |= mask;  ALPANA_GPIO_DATA = gpio_out_shadow; }
void alpana_gpio_clear(uint32_t mask)  { gpio_out_shadow &= ~mask; ALPANA_GPIO_DATA = gpio_out_shadow; }
void alpana_gpio_toggle(uint32_t mask) { gpio_out_shadow ^= mask;  ALPANA_GPIO_DATA = gpio_out_shadow; }
uint32_t alpana_gpio_read(void)        { return ALPANA_GPIO_DATA; }

void alpana_pinmux_set(int pin, int function)
{
	if (pin < PIN_JA1 || pin > PIN_JB3) return;
	pinmux_shadow &= ~(3u << (2 * pin));
	pinmux_shadow |= ((uint32_t)function & 3u) << (2 * pin);
	ALPANA_PINMUX = pinmux_shadow;
}

void alpana_delay_us(uint32_t us)
{
	uint64_t end = alpana_cycles() + (uint64_t)us * ALPANA_CYCLES_PER_US;
	while (alpana_cycles() < end) ;
}

void alpana_delay_ms(uint32_t ms) { alpana_delay_us(ms * 1000u); }

uint32_t alpana_millis(void) { return (uint32_t)(alpana_cycles() / (CLOCK_FREQUENCY / 1000)); }

uint32_t alpana_banner(void)
{
	uint32_t misa;
	asm volatile ("csrr %0, misa" : "=r"(misa));
	char ext[27]; int n = 0;
	for (int i = 0; i < 26; i++) if (misa & (1u << i)) ext[n++] = 'A' + i;
	ext[n] = 0;
	printf("\n==============================================\n");
	printf(" ALPANA_SOC  (Shakti E-class 4-stage + SP FPU)\n");
	printf(" RV32%s  @ %d MHz   DDR2 %d MB @ 0x%x\n", ext, CLOCK_FREQUENCY / 1000000,
	       TCM_MEM_SIZE >> 20, TCM_MEM_START);
	printf("==============================================\n");
	return misa;
}

/* uart_struct fields are not volatile, so at -O2 GCC would read the status
   register only once and turn a wait loop into "j ." -> always go through
   volatile pointers here. */
#define U0_STATUS (*(volatile uint8_t  *)&uart_instance[0]->status)
#define U0_TX     (*(volatile uint32_t *)&uart_instance[0]->tx_reg)

void alpana_uart_putb(uint8_t c)
{
	while (U0_STATUS & STS_TX_FULL) ;
	U0_TX = c;
}

void alpana_uart_set_baud(uint32_t baud)
{
	/* let the queued text leave before changing the divisor (bounded wait) */
	uint64_t end = alpana_cycles() + CLOCK_FREQUENCY / 10;     /* 100 ms max */
	while (!(U0_STATUS & STS_TX_EMPTY) && alpana_cycles() < end) ;
	alpana_delay_us(200);                                       /* last char shifts out */
	*(volatile uint16_t *)&uart_instance[0]->baud = (uint16_t)(CLOCK_FREQUENCY / (16 * baud));
}
