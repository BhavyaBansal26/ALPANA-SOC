/***************************************************************************
 * ALPANA SDK example : alpana_blink
 * LED chase on LD0..LD3. BTNC pauses, BTNL/BTNR change direction,
 * switch SW0 selects fast/slow. Prints the switches/buttons on change.
 * Build: make software PROGRAM=alpana_blink
 ***************************************************************************/
#include <stdint.h>
#include "uart.h"
#include "utils.h"
#include "alpana.h"

int main(void)
{
	alpana_banner();
	printf(" LED chase on LD0..LD3 | BTNC pause | BTNL/BTNR direction | SW0 speed\n");
	alpana_gpio_output(LED_ALL);
	alpana_gpio_input(SW0 | SW1 | SW2 | SW3 | SW4 | BTN_C | BTN_U | BTN_L | BTN_R);

	const uint32_t leds[4] = { LED0, LED1, LED2, LED3 };
	int pos = 0, dir = 1, paused = 0;
	uint32_t last_in = 0xFFFFFFFF;
	while (1) {
		uint32_t in = alpana_gpio_read() & (SW0 | SW1 | SW2 | SW3 | SW4 | BTN_C | BTN_U | BTN_L | BTN_R);
		if (in != last_in) {
			printf(" SW=%d%d%d%d%d  BTN C%d U%d L%d R%d\n",
			       !!(in & SW4), !!(in & SW3), !!(in & SW2), !!(in & SW1), !!(in & SW0),
			       !!(in & BTN_C), !!(in & BTN_U), !!(in & BTN_L), !!(in & BTN_R));
			if ((in & BTN_C) && !(last_in & BTN_C)) paused = !paused;
			last_in = in;
		}
		if (in & BTN_L) dir = 1;
		if (in & BTN_R) dir = -1;
		if (!paused) {
			alpana_gpio_clear(LED_ALL);
			alpana_gpio_set(leds[pos]);
			pos = (pos + dir + 4) & 3;
		}
		alpana_delay_ms((in & SW0) ? 60 : 250);
	}
	return 0;
}
