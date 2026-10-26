/***************************************************************************
 * Project           : ALPANA SDK
 * Name of the file  : alpana_spi1.c
 * Brief             : SPI1 master receive driver (see alpana_spi1.h)
 ***************************************************************************/
#include "alpana.h"
#include "alpana_spi1.h"

#define SPI_CR1   (*(volatile uint32_t *)(SPI1_START + 0x00))
#define SPI_CR2   (*(volatile uint32_t *)(SPI1_START + 0x04))
#define SPI_SR    (*(volatile uint32_t *)(SPI1_START + 0x08))
#define SPI_DR(i) (*(volatile uint32_t *)(SPI1_START + 0x0C + 4 * (i)))

void alpana_spi1_init(int br)
{
	alpana_pinmux_set(PIN_JA10, PINMUX_ALT1);   /* NSS  */
	alpana_pinmux_set(PIN_JB1,  PINMUX_ALT1);   /* MOSI */
	alpana_pinmux_set(PIN_JB2,  PINMUX_ALT1);   /* MISO */
	alpana_pinmux_set(PIN_JB3,  PINMUX_ALT1);   /* SCLK */
	SPI_CR1 = (160u << 24)                      /* TOTAL_BITS_RX = 160          */
	        | (1u << 9)                         /* SSM: software NSS ...        */
	        | (1u << 6)                         /* SPE (SSI = 0: NSS held low)  */
	        | ((uint32_t)(br & 7) << 3)         /* baud: clk / (2*(br+1))       */
	        | (1u << 2)                         /* MSTR                         */
	        | 3u;                               /* CPOL = CPHA = 1 (mode 3)     */
}

static inline __attribute__((always_inline)) void burst_start(void)
{
	SPI_SR  = 0;                                /* clear RXNE */
	SPI_CR2 = 1u << 15;                         /* RX_START (self-clearing) */
}
static inline __attribute__((always_inline)) int burst_wait(void)
{
	uint32_t n = 0;
	while (!(SPI_SR & 1u)) if (++n > 20000u) return 0;
	return 1;
}

ALPANA_FAST int alpana_spi1_read_be_words(uint32_t *dst, uint32_t len)
{
	uint32_t words = len / 4;
	if (len == 0 || len % ALPANA_SPI1_BURST) return -1;
	burst_start();
	for (uint32_t o = 0; o < words; o += 5) {
		if (!burst_wait()) return -2;
		if (o + 5 < words) burst_start();        /* next burst on the wire ...  */
		dst[o]     = SPI_DR(0);                  /* ... while this one is copied */
		dst[o + 1] = SPI_DR(1);
		dst[o + 2] = SPI_DR(2);
		dst[o + 3] = SPI_DR(3);
		dst[o + 4] = SPI_DR(4);
	}
	return 0;
}

int alpana_spi1_read(uint8_t *dst, uint32_t len)
{
	uint32_t w[5];
	if (len == 0 || len % ALPANA_SPI1_BURST) return -1;
	burst_start();
	for (uint32_t o = 0; o < len; o += ALPANA_SPI1_BURST) {
		if (!burst_wait()) return -2;
		for (int i = 0; i < 5; i++) w[i] = SPI_DR(i);
		if (o + ALPANA_SPI1_BURST < len) burst_start();
		for (int i = 0; i < 5; i++) {
			dst[o + 4 * i]     = w[i] >> 24;
			dst[o + 4 * i + 1] = w[i] >> 16;
			dst[o + 4 * i + 2] = w[i] >> 8;
			dst[o + 4 * i + 3] = w[i];
		}
	}
	return 0;
}
