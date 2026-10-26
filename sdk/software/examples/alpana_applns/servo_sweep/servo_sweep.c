/***************************************************************************
 * ALPANA SDK example : servo_sweep
 * Hobby servo on PWM1 (Pmod JA pin 7): 50 Hz, sweeps -30..+30 degrees.
 * Wiring: signal -> JA7, servo +5 V from an EXTERNAL supply,
 *         servo GND -> supply GND and Nexys GND (JA pin 5).
 * LD0 = turning left, LD3 = turning right, LD1+LD2 = centre.
 * Build: make software PROGRAM=servo_sweep
 ***************************************************************************/
#include <stdint.h>
#include "uart.h"
#include "utils.h"
#include "alpana.h"
#include "alpana_servo.h"

#define SERVO_CH 1          /* PWM1 -> JA7 */
#define RANGE    30         /* +-30 degrees */

int main(void)
{
	alpana_banner();
	printf(" servo sweep on PWM1 / JA7 (50 Hz, 1000..2000 us)\n");
	alpana_gpio_output(LED_ALL);
	alpana_servo_init(SERVO_CH);
	int deg = 0, step = 2;
	while (1) {
		uint32_t us = alpana_servo_set_angle(SERVO_CH, deg, RANGE);
		alpana_gpio_clear(LED_ALL);
		if (deg < -5) alpana_gpio_set(LED0);
		else if (deg > 5) alpana_gpio_set(LED3);
		else alpana_gpio_set(LED1 | LED2);
		if (deg % 10 == 0) printf(" angle %d deg -> pulse %u us\n", deg, us);
		deg += step;
		if (deg >= RANGE || deg <= -RANGE) step = -step;
		alpana_delay_ms(40);
	}
	return 0;
}
