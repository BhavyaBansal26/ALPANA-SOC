/***************************************************************************
 * Project           : ALPANA SDK
 * Name of the file  : alpana_servo.h
 * Brief             : PWM in microseconds + hobby-servo helpers (Shakti pwm @0x30000)
 *
 *  Verified against mkpwm_e32.v in RTL simulation: divisor 40, period 20000,
 *  duty 1500 -> exactly 20.000 ms period / 1.500 ms high at 80 MHz.
 *  Register rules (from the RTL): PERIOD(+0), DUTY(+4), CLOCK(+0xC) need
 *  16-bit stores; CONTROL(+8) needs an 8-bit store.
 *  Channel -> Pmod pin (pin-mux set automatically):
 *    PWM0 JA4   PWM1 JA7   PWM2 JA8   PWM3 JA9   PWM4 JA10   PWM5 JB1
 *  (PWM4/PWM5 share JA10/JB1 with SPI1 NSS/MOSI.)
 ***************************************************************************/
#ifndef ALPANA_SERVO_H
#define ALPANA_SERVO_H
#include <stdint.h>

/* 1 us resolution PWM: period 1..65535 us (e.g. 20000 = 50 Hz) */
void alpana_pwm_init_us(int ch, uint32_t period_us, uint32_t high_us);
void alpana_pwm_set_us(int ch, uint32_t high_us);
void alpana_pwm_stop(int ch);

/* hobby servo: 50 Hz, 1000..2000 us (1500 = centre) */
void alpana_servo_init(int ch);
void alpana_servo_set_us(int ch, uint32_t us);                 /* clamped to 500..2500 */
/* deg in -range_deg..+range_deg  ->  1000..2000 us (e.g. range 30 for steering) */
uint32_t alpana_servo_set_angle(int ch, int deg, int range_deg);
#endif
