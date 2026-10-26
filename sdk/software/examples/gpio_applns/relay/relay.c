/***************************************************************************
 * Project                           : shakti devt board
 * Name of the file	             : relay.c
 * Brief Description of file         : Helps to on and off the relay through gpio pins
 * Name of Author                    : Soutrick Roy Chowdhury
 * Email ID                          : soutrick97@gmail.com

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
@file relay.c
@brief Implements gpio functionality.
@detail Helps to on and off the relay within a certain interval.
*/

#include "platform.h"
#include "gpio.h"
#include "utils.h"

/** @fn void main()
 * @brief writes the data to the output register based on control word with a delay
 */
void main()
{
    write_word(GPIO_DIRECTION_CNTRL_REG, 0x00000001);
    
    while (1) {
        write_word(GPIO_DATA_REG, 0x00000001);
        delay_loop(1000,5000);
        write_word(GPIO_DATA_REG, 0x00);
        delay_loop(1000,5000);

        printf("\nWORKING");
    }
    
}
