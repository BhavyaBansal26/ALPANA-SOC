/************************************************************************
 * Project           		:  Shakti development board
 * Name of the file	     	:  Intruder detection.c
 * Brief Description of file    :  This is a program for Intruder Detection System which uses PIR Sensor to detect any intrusion
 * Name of the author           :  Sambhav Jain 
 * Email ID                     :  sambhav.jv@gmail.com
    Copyright (C) 2020  IIT Madras. All rights reserved.

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
@file intruder_detection.h
@brief Intruder detection using PIR Sensors and Keypad 
@detail Intruder detection using PIR Sensors for detection of intruder and Keypad for interfacing, locking and unlocking the device.
*/


#ifndef INTRUDER_DETECTION_H
#define INTRUDER_DETECTION_H

#define ROWNUM              4
#define COLNUM              4
#define OUTPINS_OFFSET      0
#define INPINS_OFFSET       4
#define OUTPINS             (0xF << OUTPINS_OFFSET)
#define INPINS              (0xF << INPINS_OFFSET)

//Peripheral Pins
#define BUZZER              GPIO11
#define LED_P1              GPIO16
#define LED_P2              GPIO18
#define LED_P3              GPIO19
#define LED_P4              GPIO21
#define POWER_SWITCH        GPIO24
#define DEACTIVATE_SWITCH   GPIO25
#define PANIC_BUTTON        GPIO28          //Input 
#define GPIO_OUTPINS        (LED_P1 | LED_P2 | LED_P3 | LED_P4 | BUZZER)
#define PIR_IN	            GPIO8
#define PANIC_BUTTON_OFFSET 28
#define PIR_OFFSET          8
#define ALARM_TIMER         5
#define HIGH                1
#define LOW                 0

//Keypad 
#define PASS_NUM            4               //Number of Digits Password

//function prototype
void set_key_pins_map(unsigned int* row, unsigned int* col, int table[COLNUM][ROWNUM]);
int get_key();
int get_col_row(void);
unsigned int row_pins[ROWNUM];
unsigned int col_pins[COLNUM];
int key_map[COLNUM][ROWNUM];

#endif
