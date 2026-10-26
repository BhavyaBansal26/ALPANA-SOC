/***************************************************************************
* Project               	        	:  shakti devt board
* Name of the file	            	    :  pwmled.c
* Brief Description of file             :  example file for pwm led.
* Name of Author    	                :  Abhinav Ramnath
* Email ID                              :  abhinavramnath13@gmail.com

 Copyright (C) 2019  IIT Madras. All rights reserved.

 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <https://www.gnu.org/licenses/>.

***************************************************************************/
/**
@file pwmled.c
@brief example file to make the led blink at specified frequency
@detail it runs the led and causes it to blink at specified frequency using the pwm module
*/

#include "pwm_driver.h"
#include "pinmux.h"

/** @fn int main()
 * @brief main function that runs the code
 * @return zero
 */
int main()
{
	/**we need to set the period, duty cycle and the clock divisor in order
	 *to set it to the frequency required. Base clock is 50MHz
	 **/
	pwm_configure(PWM_0, 0xf000, 0xf0, 0x80, false);
    *pinmux_config_reg = 0x80;
		
	/*This function starts the PWM in the specified mode*/
	pwm_start(PWM_0,0);
	return 0;
}
