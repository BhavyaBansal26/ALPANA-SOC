/***************************************************************************
 * Project           	           : shakti devt board
 * Name of the file	           : motor.c
 * Brief Description of file       : Determines the functioning of stepper motor with the  help of gpio pins.  
 * Name of Author    	           : Sathya Narayanan N
 * Email ID                        : sathya281@gmail.com

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
@file motor.c
@brief Determines the functioning of stepper motor with the  help of gpio pins.
@detail This file contains the program to control a simple stepper motor using
shakti. The demonstration is done using the gpio pins.
*/

#include <stdio.h>/* Includes definitions of standard input/output functions*/
#include "gpio.h"/*Incudes the defintions of gpio functions and registers*/
#include "platform.h"

#define MOTOR_FORWARD_CNTRL (0xA << 0)/*Defines the Required Forward control word*/
#define MOTOR_REVERSE_CNTRL (0x5 << 0)/*Defines the Required Reverse control word*/

/** @fn void main()
 * @brief Performs the intilization of the corresponding registers of gpio pins.
 */
void main()
{
	log_info("Motor Control\n");
	write_word(GPIO_DIRECTION_CNTRL_REG, 0xFFFFFFFF);

	while(1) {
		write_word(GPIO_DATA_REG, MOTOR_FORWARD_CNTRL);
		log_info("Forward\n");
		log_info("GPIO Data Register Value: %d\n",GPIO_DATA_REG);
		delay_loop(5000, 5000);

		write_word(GPIO_DATA_REG, MOTOR_REVERSE_CNTRL);
		log_info("Reverse\n");
		log_info("GPIO Data Register Value: %d\n",GPIO_DATA_REG);
		delay_loop(5000, 5000);
		}

	delay_loop(5000, 5000);
}
