/***************************************************************************
 * ALPANA SDK example : alpana_hello
 * Prints the SoC banner, checks mcycle against the CLINT timer,
 * exercises the hardware FPU and reports CPI of a small loop.
 * Build: make software PROGRAM=alpana_hello
 ***************************************************************************/
#include <stdint.h>
#include "uart.h"
#include "utils.h"
#include "alpana.h"

static uint64_t mtime_now(void)
{
	volatile uint32_t *mt = (volatile uint32_t *)MTIME;
	uint32_t hi, lo;
	do { hi = mt[1]; lo = mt[0]; } while (hi != mt[1]);
	return ((uint64_t)hi << 32) | lo;
}

int main(void)
{
	alpana_banner();

	/* timers: 100 ms by mcycle should be 31250 CLINT ticks (mtime = clk / 256) */
	uint64_t t0 = mtime_now();
	alpana_delay_ms(100);
	printf(" 100 ms delay = %u mtime ticks (expect %u)\n", (uint32_t)(mtime_now() - t0),
	       CLOCK_FREQUENCY / CLINT_DIVISOR / 10);

	/* FPU */
	volatile float a = 1.5f, b = 2.25f;
	float r = a * b + a / b;                           /* 3.375 + 0.6667 = 4.0417 */
	int frac = (int)((r - (int)r) * 10000.0f);         /* 4 decimals, zero padded */
	printf(" FPU: 1.5*2.25 + 1.5/2.25 = %d.%d%d%d%d (expect 4.0416)\n", (int)r,
	       frac / 1000, (frac / 100) % 10, (frac / 10) % 10, frac % 10);

	/* CPI of a simple loop (code runs from DDR2: expect a high CPI, no cache) */
	uint64_t i0 = alpana_instret(), k0 = alpana_cycles();
	volatile uint32_t acc = 0;
	for (uint32_t i = 0; i < 1000; i++) acc += i;
	uint64_t ni = alpana_instret() - i0, nc = alpana_cycles() - k0;
	printf(" loop: %u instructions in %u cycles -> CPI %u.%u\n", (uint32_t)ni, (uint32_t)nc,
	       (uint32_t)(nc / ni), (uint32_t)((nc * 10 / ni) % 10));

	printf(" Hello from ALPANA_SOC!\n");
	return 0;                                          /* init() parks the core in boot RAM */
}
