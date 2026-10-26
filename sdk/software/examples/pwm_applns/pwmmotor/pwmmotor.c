/***************************************************************************
* Project               	  : shakti devt board
* Name of the file	          : pwmmotor.c
* Brief Description of file       : example file for controlling motor via pwm .
* Name of Author    	          : Abhinav Ramnath
* Email ID                        : abhinavramnath13@gmail.com

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
@file pwmmotor.c
@brief example file to run the pwm motor at a specified frequency
@detail it runs the motor using pwm module at specified frequency 
*/

#include "pwm_driver.h"

/** @fn int main()
 * @brief main function that runs the code
 * @details runs the code 
 * @return zero
 */
int main()
{
	/**we need to set the period, duty cycle and the clock divisor in order
	 *to set it to the frequency required. Base clock is 50MHz
	 **/
	pwm_configure(PWM_0, 0xffff, 0xff, 0x01, false);

	/*This function starts the PWM in the specified mode*/
	pwm_start(PWM_0,2);

	while(1);

	return 0;
}
