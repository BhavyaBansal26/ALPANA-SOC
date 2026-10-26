/***************************************************************************
 * Project           		: shakti devt board
 * Name of the file	     	: passive_buzzer.c
 * Brief Description of file    : It will buzzer at a desired frequency when you give oscillating electronic signal.
 * Name of Author               : Soutrick Roy Chowdhury
 * Email ID                     : soutrick97@gmail.com

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

*****************************************************************************/
/**
@file passive_buzzer.c
@brief Implements gpio functionality
@detail It will buzzer at a desired frequency when you give an oscillating electronic signal.
*/

#include "platform.h"
#include "gpio.h"

/** @fn void main()
 * @brief A passive buzzer (AKA magnetic transducer) can make different tones,
 * but the devices that controls the buzzer has to provide it with an oscillating
 * electronic signal at a desired frequency. The supplied frequency will determine 
 * the tone. Supplying just a fixed voltage will generate no sound, except perhaps
 * a slight "tick" at the point when the power source is connected or disconnected from the buzzer.
 */
void main()
{
    // Assume 0 for Input and 1 for Output
    write_word(GPIO_DIRECTION_CNTRL_REG, GPIO0);	//Connect in a Bit 0
    int i = 0;

    while (1) {
        for (i = 0; i < 80; i++) {		       //When a frequency sound 
            write_word(GPIO_DATA_REG, GPIO0);	       //Sends tone
            delay_loop(1000,1000);
            write_word(GPIO_DATA_REG, 0x00);		//No tone
            delay_loop(1000,1000);
        }

        for (i = 0; i < 80; i++) {
            write_word(GPIO_DATA_REG, GPIO0);
            delay_loop(1000,1000);
            write_word(GPIO_DATA_REG, 0x00);
            delay_loop(1000,1000);
        }
        
    }
}
    
