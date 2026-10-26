/***************************************************************************
* Project           		: shakti devt board
* Name of the file	     	: switches.c
* Brief Description of file     : Read the value from the switches
* Name of Author    	        : Madan Kumar S
* Email ID                      : kumarmadan96@gmail.com

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
@file switches.c
@brief Read the value from the switches
@detail 
*/

#include <stdio.h> //includes the definitions of standard input/output functions
#include "platform.h"
#include "gpio.h" // includes definitions of gpio pins and read, write functions//
#include "switch_driver.h"
#include "utils.h"

/*! Define one of the the following macros to read buttons in different ways. */
#define READ_INDIVIDUAL_SWITCHES /*! Read each switch value */
//#define READ_ALL_SWITCHES /*! Read All switch values */

unsigned long read_data = 0;

/** @fn  void main() 
 * @brief Performs the toggling operation  with the help of button.
 * @details 
 * @warning 
 */
void main()
{

#if defined(ARTIX7_35T) || defined(ARTIX7_100T)


	while(1){	
#ifdef READ_INDIVIDUAL_SWITCHES		
		/*! Define any one button from below at a time */
#define SW_0 /*! Read SW0 value */
		//#define SW_1 /*! Read SW1 value */
		//#define SW_2 /*! Read SW2 value */
		//#define SW_3 /*! Read SW3 value */

#ifdef SW_0
		configure_switch(SW0);	
#endif	

#ifdef SW_1
		configure_switch(SW1);	
#endif	

#ifdef SW_2
		configure_switch(SW2);	
#endif	

#ifdef SW_3
		configure_switch(SW3);	
#endif				

#endif

#ifdef READ_ALL_SWITCHES	

		configure_all_switch();	

#endif

#ifdef READ_INDIVIDUAL_SWITCHES	

#ifdef SW_0
		read_data = read_word(GPIO_DATA_REG); /* Copies the GPIO_DATA_REG Register contents */
		printf("\n Read Data is :0x%x", read_data);
		read_data = (read_data & 0x01000000);
		printf("\n Read Data is :0x%x", read_data);
		if(read_data == 0x01000000)
			printf("\n SW0: HIGH"); 		
		else
			printf("\n SW0: LOW");
#endif

#ifdef SW_1
		read_data = read_word(GPIO_DATA_REG);
		printf("\n Read Data is :0x%x", read_data);
		read_data = (read_data & 0x02000000);
		printf("\n Read Data is :0x%x", read_data);
		if(read_data == 0x02000000)
			printf("\n SW1: HIGH"); 		
		else
			printf("\n SW1: LOW");
#endif

#ifdef SW_2
		read_data = read_word(GPIO_DATA_REG);
		printf("\n Read Data is :0x%x", read_data);
		read_data = (read_data & 0x04000000);
		printf("\n Read Data is :0x%x", read_data);
		if(read_data == 0x04000000)
			printf("\n SW2: HIGH"); 		
		else
			printf("\n SW2: LOW");
#endif

#ifdef SW_3
		read_data = read_word(GPIO_DATA_REG);
		printf("\n Read Data is :0x%x", read_data);
		printf("\n Read Data is :0x%x", read_data);
		if(read_data == 0x08000000)
			printf("\n SW3: HIGH"); 		
		else
			printf("\n SW3: LOW");
#endif

		delay_loop(500, 500);

#endif

#ifdef READ_ALL_SWITCHES	

		printf("\n Read Data is :0x%x", read_data);
		read_data = read_word(GPIO_DATA_REG);
		printf("\n Read Data is :0x%x", read_data);

		switch(read_data)
		{

		case 0x00000000:
			printf("\n SW0: LOW");
			printf("\n SW1: LOW");
			printf("\n SW2: LOW");
			printf("\n SW3: LOW");
			break;

		case 0x01000000:
			printf("\n SW0: HIGH");
			printf("\n SW1: LOW");
			printf("\n SW2: LOW");
			printf("\n SW3: LOW");
			break;

		case 0x02000000:
			printf("\n SW0: LOW");
			printf("\n SW1: HIGH");
			printf("\n SW2: LOW");
			printf("\n SW3: LOW");
			break;

		case 0x03000000:
			printf("\n SW0: HIGH");
			printf("\n SW1: HIGH");
			printf("\n SW2: LOW");
			printf("\n SW3: LOW");
			break;

		case 0x04000000:
			printf("\n SW0: LOW");
			printf("\n SW1: LOW");
			printf("\n SW2: HIGH");
			printf("\n SW3: LOW");
			break;

		case 0x05000000:
			printf("\n SW0: HIGH");
			printf("\n SW1: LOW");
			printf("\n SW2: HIGH");
			printf("\n SW3: LOW");
			break;

		case 0x06000000:
			printf("\n SW0: LOW");
			printf("\n SW1: HIGH");
			printf("\n SW2: HIGH");
			printf("\n SW3: LOW");
			break;

		case 0x07000000:
			printf("\n SW0: HIGH");
			printf("\n SW1: HIGH");
			printf("\n SW2: HIGH");
			printf("\n SW3: LOW");
			break;

		case 0x08000000:
			printf("\n SW0: LOW");
			printf("\n SW1: LOW");
			printf("\n SW2: LOW");
			printf("\n SW3: HIGH");
			break;

		case 0x09000000:
			printf("\n SW0: HIGH");
			printf("\n SW1: LOW");
			printf("\n SW2: LOW");
			printf("\n SW3: HIGH");
			break;

		case 0x0A000000:
			printf("\n SW0: LOW");
			printf("\n SW1: HIGH");
			printf("\n SW2: LOW");
			printf("\n SW3: HIGH");
			break;

		case 0x0B000000:
			printf("\n SW0: HIGH");
			printf("\n SW1: HIGH");
			printf("\n SW2: LOW");
			printf("\n SW3: HIGH");
			break;

		case 0x0C000000:
			printf("\n SW0: LOW");
			printf("\n SW1: LOW");
			printf("\n SW2: HIGH");
			printf("\n SW3: HIGH");
			break;

		case 0x0D000000:
			printf("\n SW0: HIGH");
			printf("\n SW1: LOW");
			printf("\n SW2: HIGH");
			printf("\n SW3: HIGH");
			break;

		case 0x0E000000:
			printf("\n SW0: LOW");
			printf("\n SW1: HIGH");
			printf("\n SW2: HIGH");
			printf("\n SW3: HIGH");
			break;

		case 0x0F000000:
			printf("\n SW0: HIGH");
			printf("\n SW1: HIGH");
			printf("\n SW2: HIGH");
			printf("\n SW3: HIGH");
			break;

		}

		delay_loop(500, 500);

#endif
	}	
	delay_loop(500, 500);

#endif
}
