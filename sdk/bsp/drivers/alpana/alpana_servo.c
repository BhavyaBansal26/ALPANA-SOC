/***************************************************************************
 * Project           : ALPANA SDK
 * Name of the file  : alpana_servo.c
 * Brief             : PWM (1 us ticks) and servo helpers (see alpana_servo.h)
 ***************************************************************************/
#include "alpana.h"
#include "alpana_servo.h"

#define PWM_REG16(ch, off) (*(volatile uint16_t *)(PWM_BASE_ADDRESS + (ch) * PWM_MODULE_OFFSET + (off)))
#define PWM_REG8(ch, off)  (*(volatile uint8_t  *)(PWM_BASE_ADDRESS + (ch) * PWM_MODULE_OFFSET + (off)))
#define PWM_PERIOD 0x0
#define PWM_DUTY   0x4
#define PWM_CTRL   0x8
#define PWM_CLOCK  0xC
#define CTRL_ENABLE  0x02
#define CTRL_START   0x04
#define CTRL_OUT_EN  0x10

static const int pwm_pin[6] = { PIN_JA4, PIN_JA7, PIN_JA8, PIN_JA9, PIN_JA10, PIN_JB1 };

void alpana_pwm_init_us(int ch, uint32_t period_us, uint32_t high_us)
{
	if (ch < 0 || ch > 5) return;
	if (period_us > 65535) period_us = 65535;
	if (high_us > period_us) high_us = period_us;
	alpana_pinmux_set(pwm_pin[ch], PINMUX_ALT2);
	PWM_REG16(ch, PWM_CLOCK)  = (uint16_t)(CLOCK_FREQUENCY / 2000000);  /* 1 MHz tick: clk/(2*div) */
	PWM_REG16(ch, PWM_PERIOD) = (uint16_t)period_us;
	PWM_REG16(ch, PWM_DUTY)   = (uint16_t)high_us;
	PWM_REG8(ch, PWM_CTRL)    = CTRL_ENABLE | CTRL_START | CTRL_OUT_EN;
}

void alpana_pwm_set_us(int ch, uint32_t high_us)
{
	if (ch < 0 || ch > 5) return;
	PWM_REG16(ch, PWM_DUTY) = (uint16_t)high_us;
}

void alpana_pwm_stop(int ch)
{
	if (ch < 0 || ch > 5) return;
	PWM_REG8(ch, PWM_CTRL) = 0;
}

void alpana_servo_init(int ch) { alpana_pwm_init_us(ch, 20000, 1500); }

void alpana_servo_set_us(int ch, uint32_t us)
{
	if (us < 500) us = 500;
	if (us > 2500) us = 2500;
	alpana_pwm_set_us(ch, us);
}

uint32_t alpana_servo_set_angle(int ch, int deg, int range_deg)
{
	if (range_deg <= 0) range_deg = 90;
	if (deg > range_deg) deg = range_deg;
	if (deg < -range_deg) deg = -range_deg;
	uint32_t us = (uint32_t)(1500 + deg * 500 / range_deg);
	alpana_servo_set_us(ch, us);
	return us;
}
