/***************************************************************************
 * Project           	        : shakti devt board
 * Name of the file	        : btnled.c
 * Brief Description of file    : Controls the led operation with help of button,gpio based.
 * Name of Author    	        : Sathya Narayanan N & Raghav
 * Email ID                     : sathya281@gmail.com

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
@file btnled.c
@brief Controls the led operation with help of button,gpio based.
@detail A led glows when a button is pressed. The button controlled led is
implemented using gpio pins.
*/

#include "gpio.h"
#include "platform.h"

/** @fn int main(void)
 * @brief on press of a button led grows. The technique used here is polling
 *    based.
 * @return int
 */
int main(void)
{
	write_word(GPIO_DIRECTION_CNTRL_REG, ~(1 << 0) );

	while (1) {

		unsigned long readData = 0;

		readData = read_word(GPIO_DATA_REG) & 0x1;    

		log_debug("\n Read Data is :0x%08lx", readData);

		if (readData) {           //if readdata //
			log_debug("; LED ON");
			write_word(GPIO_DATA_REG, 0X2); 
		}
		else {                    //if not //
			log_debug("; LED OFF");
			write_word(GPIO_DATA_REG, 0X0); 
		}
		delay_loop(1000,1000);
	}

	return 0;
}
