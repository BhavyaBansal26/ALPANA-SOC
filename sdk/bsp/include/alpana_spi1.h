/***************************************************************************
 * Project           : ALPANA SDK
 * Name of the file  : alpana_spi1.h
 * Brief             : SPI1 master RECEIVE driver (Shakti spi block @0x20100)
 *
 *  Verified against the ALPANA_SOC RTL (mkspi_e32.v) in simulation:
 *   - SPI mode 3 (CPOL=1, CPHA=1), MSB first, 20 bytes per hardware burst
 *   - CR1: RX bits=160, SSM=1/SSI=0 (internal NSS held low), SPE, MSTR, BR
 *   - CR2.RX_START (bit 15) starts a burst and clears itself
 *   - SR.RXNE (bit 0) = DR1..DR5 hold the 20 bytes, big-endian
 *   - DR1..DR5 stay valid until the NEXT burst completes, so the driver
 *     starts burst n+1 before copying burst n (0 errors in RTL sim)
 *  SCLK = CLOCK_FREQUENCY / (2 * (br + 1)): br 7 -> 5 MHz, br 3 -> 10 MHz.
 *  Chip select: drive it from any GPIO (e.g. JD2) so a slave sees one
 *  continuous transfer across bursts. Transmit through this block is not
 *  supported by this driver (use bsp/drivers/spi/gpio_spi.c for writes).
 *  Pins: JB3 = SCLK, JB2 = MISO, JB1 = MOSI, JA10 = NSS (unused).
 ***************************************************************************/
#ifndef ALPANA_SPI1_H
#define ALPANA_SPI1_H
#include <stdint.h>

#define ALPANA_SPI1_BURST 20                       /* bytes per hardware burst */

void alpana_spi1_init(int br);                     /* pin-mux + mode 3 receive config */
/* receive len bytes (len multiple of 20) in stream order; dst any alignment */
int  alpana_spi1_read(uint8_t *dst, uint32_t len);
/* fastest path: len multiple of 20, dst 4-byte aligned, stored as RAW
   big-endian words (byte k of the stream = (w[k/4] >> (24 - 8*(k%4))) & 0xFF) */
int  alpana_spi1_read_be_words(uint32_t *dst, uint32_t len);
static inline uint32_t alpana_bswap32(uint32_t w) {
  return (w >> 24) | ((w >> 8) & 0xFF00u) | ((w << 8) & 0xFF0000u) | (w << 24);
}
#endif
