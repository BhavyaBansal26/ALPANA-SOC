/***************************************************************************
 * Project           		: shakti devt board
 * Name of the file	     	: buttons.c
 * Brief Description of file    : Read the value from the switches
 * Name of Author    	        : Madan Kumar S
 * Email ID                     : kumarmadan96@gmail.com

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
@file  buttons.c
@brief Contains the driver routines to read the pressed button.
@detail Detects the pressed key/button and prints pressed button.
*/

#include <stdio.h>        
#include "platform.h"
#include "gpio.h"         
#include "button_driver.h"
#include "utils.h"

/*! Define one of the the following macros to read buttons in different ways. */
#define READ_INDIVIDUAL_BTNS /*! Read each button value */
//#define READ_ALL_BTNS /*! Read All button values */

unsigned long read_data = 0;

/** @fn void main() 
 * @brief Performs the toggling operation  with the help of button.
 * @details Configures the buttons to be read as inputs and 
 *          displays the pressed button value.
 */
void main()
{

#if defined(ARTIX7_35T) || defined(ARTIX7_100T)

	while (1) {

#ifdef READ_INDIVIDUAL_BTNS

		/*! Define any one button from below at a time */
#define BTN_0  /*! Read BTN0 value */
		//#define BTN_1 /*! Read BTN1 value */
		//#define BTN_2 /*! Read BTN2 value */
		//#define BTN_3 /*! Read BTN3 value */

#ifdef BTN_0
		configure_btn(BTN0);
#endif

#ifdef BTN_1
		configure_btn(BTN1);
#endif

#ifdef BTN_2
		configure_btn(BTN2);
#endif

#ifdef BTN_3
		configure_btn(BTN3);
#endif

#endif

#ifdef READ_ALL_BTNS

		configure_all_btn();

#endif

#ifdef READ_INDIVIDUAL_BTNS

#ifdef BTN_0
		read_data = read_word(GPIO_DATA_REG); /* Copies the GPIO_DATA_REG Register contents */
		printf("\n Read Data is :0x%x", read_data);
		read_data = (read_data & 0x10000000);
		printf("\n Read Data is :0x%x", read_data);
		if(read_data == 0x10000000)
			printf("\n BTN0: HIGH"); 		
		else
			printf("\n BTN0: LOW");
#endif

#ifdef BTN_1
		read_data = read_word(GPIO_DATA_REG);
		printf("\n Read Data is :0x%x", read_data);
		read_data = (read_data & 0x20000000);
		printf("\n Read Data is :0x%x", read_data);
		if(read_data == 0x20000000)
			printf("\n BTN1: HIGH"); 		
		else
			printf("\n BTN1: LOW");
#endif

#ifdef BTN_2
		read_data = read_word(GPIO_DATA_REG);
		printf("\n Read Data is :0x%x", read_data);
		read_data = (read_data & 0x40000000);
		printf("\n Read Data is :0x%x", read_data);
		if(read_data == 0x40000000)
			printf("\n BTN2: HIGH"); 		
		else
			printf("\n BTN2: LOW");
#endif

#ifdef BTN_3
		read_data = read_word(GPIO_DATA_REG);
		printf("\n Read Data is :0x%x", read_data);
		read_data = (read_data & 0x80000000);
		printf("\n Read Data is :0x%x", read_data);

		if(read_data == 0x80000000)
			printf("\n BTN3: HIGH"); 		
		else
			printf("\n BTN3: LOW");
#endif

		delay_loop(500, 500);

#endif

#ifdef READ_ALL_BTNS	

		printf("\n Read Data is :0x%x", read_data);
		read_data = read_word(GPIO_DATA_REG);
		printf("\n Read Data is :0x%x", read_data);

		switch(read_data)
		{

		case 0x00000000:
			printf("\n BTN0: LOW");
			printf("\n BTN1: LOW");
			printf("\n BTN2: LOW");
			printf("\n BTN3: LOW");
			break;

		case 0x10000000:
			printf("\n BTN0: HIGH");
			printf("\n BTN1: LOW");
			printf("\n BTN2: LOW");
			printf("\n BTN3: LOW");
			break;

		case 0x20000000:
			printf("\n BTN0: LOW");
			printf("\n BTN1: HIGH");
			printf("\n BTN2: LOW");
			printf("\n BTN3: LOW");
			break;

		case 0x30000000:
			printf("\n BTN0: HIGH");
			printf("\n BTN1: HIGH");
			printf("\n BTN2: LOW");
			printf("\n BTN3: LOW");
			break;

		case 0x40000000:
			printf("\n BTN0: LOW");
			printf("\n BTN1: LOW");
			printf("\n BTN2: HIGH");
			printf("\n BTN3: LOW");
			break;

		case 0x50000000:
			printf("\n BTN0: HIGH");
			printf("\n BTN1: LOW");
			printf("\n BTN2: HIGH");
			printf("\n BTN3: LOW");
			break;

		case 0x60000000:
			printf("\n BTN0: LOW");
			printf("\n BTN1: HIGH");
			printf("\n BTN2: HIGH");
			printf("\n BTN3: LOW");
			break;

		case 0x70000000:
			printf("\n BTN0: HIGH");
			printf("\n BTN1: HIGH");
			printf("\n BTN2: HIGH");
			printf("\n BTN3: LOW");
			break;

		case 0x80000000:
			printf("\n BTN0: LOW");
			printf("\n BTN1: LOW");
			printf("\n BTN2: LOW");
			printf("\n BTN3: HIGH");
			break;

		case 0x90000000:
			printf("\n BTN0: HIGH");
			printf("\n BTN1: LOW");
			printf("\n BTN2: LOW");
			printf("\n BTN3: HIGH");
			break;

		case 0xA0000000:
			printf("\n BTN0: LOW");
			printf("\n BTN1: HIGH");
			printf("\n BTN2: LOW");
			printf("\n BTN3: HIGH");
			break;

		case 0xB0000000:
			printf("\n BTN0: HIGH");
			printf("\n BTN1: HIGH");
			printf("\n BTN2: LOW");
			printf("\n BTN3: HIGH");
			break;

		case 0xC0000000:
			printf("\n BTN0: LOW");
			printf("\n BTN1: LOW");
			printf("\n BTN2: HIGH");
			printf("\n BTN3: HIGH");
			break;

		case 0xD0000000:
			printf("\n BTN0: HIGH");
			printf("\n BTN1: LOW");
			printf("\n BTN2: HIGH");
			printf("\n BTN3: HIGH");
			break;

		case 0xE0000000:
			printf("\n BTN0: LOW");
			printf("\n BTN1: HIGH");
			printf("\n BTN2: HIGH");
			printf("\n BTN3: HIGH");
			break;

		case 0xF0000000:
			printf("\n BTN0: HIGH");
			printf("\n BTN1: HIGH");
			printf("\n BTN2: HIGH");
			printf("\n BTN3: HIGH");
			break;
		}

		delay_loop(500, 500);
#endif
	}
	delay_loop(500, 500);
#endif
}

