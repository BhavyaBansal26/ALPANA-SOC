/***************************************************************************
* Project				:  shakti devt board
* Name of the file			:  intruder_detection.c
* Brief Description of file		:  Intruder detection with keypad and PIR sensors.
* Name of Author			:  Sambhav Jain
* Email ID				:  sambhav.jv@gmail.com 
   
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
@file intruder_detection.c
@brief Intruder detection using PIR Sensors and Keypad 
@detail Intruder detection using PIR Sensors for detection of intruder and Keypad for interfacing, locking and unlocking the device.
*/

/***********************************************************
*		Pin Definitions
************************************************************/
// GPIOx       - Pin Mapped
//     	0       -Keypad Top Line-4   	(Output)
//		1       -Keypad Top Line-3		(Output)
// 		2       -Keypad Top Line-2		(Output)
//		3       -Keypad Top Line-1		(Output)
//		4       -Keypad Bottom Line-4	(Output)
//		5       -Keypad Bottom Line-3	(Output)
//		6       -Keypad Bottom Line-2	(Output)
//		7       -Keypad Bottom Line-1	(Output)
//		Gnd		-Keypad Bottom Line-5	
//		Vcc		-Keypad Bottom Line-6	
 

//		8		-PIR Sensor Output Pin	(Input)
//		16		-LED P1					(Output)
//		19		-LED P2					(Output)
//		11		-Buzzer					(Output)
//		18		-LED P3					(Output)
//		21		-LED P4					(Output)
//		28		-Panic Button			(Input)
//		24		-Power Switch   		(Input)
//		25		-Deactivation Switch	(Input)

/***********************************************************
*		Include File Definitions
************************************************************/
#include "gpio.h"
#include "platform.h"
#include "intruder_detection.h" 
#include "log.h" 
/* global variable definition*/

int pir_present_state = 0;
int pir_previous_state = 0;

int panic_present_state=0;
int panic_previous_state=0;

int alarm_present_state=0;
int alarm_previous_state=0;

int first_flag = 1; 
int alarm_flag = 0;
int deactivate_flag = 0;
int alarm_counter_flag=1; //Default value 1
int keypad_password_flag=0;

int alarm_counter = 0;
int keypad_counter= 0;

// initialize key and colrow to -1 indicate no key is pressed
int key = -1;
int col_row = -1;

unsigned int row_pins[ROWNUM];
unsigned int col_pins[COLNUM];
unsigned int col[4]={0, 1, 2, 3};
unsigned int row[4]={4, 5, 6, 7};
int key_map[COLNUM][ROWNUM];
//Password
int pass_data[PASS_NUM];

int  keyTable[4][4] =
{
	{  1,  4,  7,  0},
	{  2,  5,  8, 15},
	{  3,  6,  9, 14},
	{ 10, 11, 12, 13}
};

/** @fn write_gpio
 * @brief This function Writes to the Data Register 
 * @details Writing to specific GPIO Pins to set to HIGH or LOW 
 * @param  gpio_pin The pin number for which the function sets HIGH or LOW
 * @param gpio_pin_mode Mode = 0 - Sets the GPIO Pin to LOW, 1- Sets the GPIO Pin to HIGH 
 */
void write_gpio(unsigned long gpio_pin, int gpio_pin_mode )
{
	unsigned long read_data = 0;
	read_data = read_word(GPIO_DATA_REG);

	if(gpio_pin_mode == 1)
	{
		write_word (GPIO_DATA_REG, ( read_data | (gpio_pin ) ) );
	}
	else
	{
                write_word(GPIO_DATA_REG, ( read_data & ( ~gpio_pin ) ) );  
	}

}

/** @fn pir_read_state
 * @brief Reads the State of the PIR Sensor Output 
 * @details Reads the status of the PIR Sensor and returns its state 
 * @param temp_read_data Present Register Data of GPIO 
 * @return 0- State (1->0 or 0->0); 1- State (0->1);2- State (1->1) 
 */
int pir_read_state(unsigned long temp_read_data)
{
  pir_present_state= ((PIR_IN & temp_read_data) >>PIR_OFFSET);  

  if( (pir_present_state == 1) && (pir_previous_state == 0) ){
	//State of 0->1
   	log_debug("\nHuman Detected - Intrusion Alert %x -- %x ",pir_present_state,pir_previous_state);
	pir_previous_state = pir_present_state;
    return 1;
  }
  else if((pir_present_state == 1) && (pir_previous_state == 1)) {
	//State of 1 -> 1
	log_debug("\nPIR Code 2 %x -- %x ",pir_present_state,pir_previous_state);
	pir_previous_state = pir_present_state;
	return 2;
  }
  else {
	//State of 1->0 and 0->0
   	log_debug("\nPIR Code 0 %x -- %x",pir_present_state,pir_previous_state);
    pir_previous_state = pir_present_state;
    return 0;
  }
}


/** @fn panic_button_read_state
 * @brief Reads the State of the Panic Button 
 * @details Reads the state of the panic button and returns the panic 
 * @param temp_read_data Present Register Data of GPIO 
 * @return 0 - State (0->0 & 1->0) ;1 - State (0->1);2 - State (1->1) 
 */
int panic_button_read_state(unsigned long temp_read_data)
{
  panic_present_state= ( (PANIC_BUTTON & temp_read_data) >> PANIC_BUTTON_OFFSET);  

  if( (panic_present_state == 1) && (panic_previous_state == 0) ) {
	  log_debug("\nHuman Detected - Intrusion Alert %x -- %x ",pir_present_state,pir_previous_state);
	  log_debug("\nSOS -Panic Button Activated");
	  panic_previous_state = panic_present_state;
	  return 1;
  }
  else if( (panic_previous_state == 1) && (panic_present_state == 1) ) {
	  //Function Returns 2 when mode is 1-1 (present and prev state)
	  log_debug("\nPanic -Code 2 %x -- %x ",pir_present_state,pir_previous_state);
	  panic_previous_state = panic_present_state;
	  return 2;
  }
  else {
	  log_debug("\nPanic -Code 0 %x -- %x",pir_present_state,pir_previous_state);
	  panic_previous_state = panic_present_state;
	  return 0;
  }
}

/** @fn buzzer_tone
 * @brief Outputs the Buzzer Tone
 * @details Generates a tone for the buzzer module
 */
void buzzer_tone()
{
	int i=0;

	for (i = 0; i < 4; i++) {
		write_gpio(BUZZER,HIGH);	//Buzzer generates sound
		delay(1);
		write_gpio(BUZZER,LOW);		//No tone
		delay(1);
    }
}


/** @fn set_pins
 * @brief This function sets the pins for the row and column
 * 			and maps the table to its keymap
 * @details This function sets the mapping of pin for row and column. 
 * @param row pointer address which sets row pins   
 * @param col pointer address which sets column pins
 * @param table Array which maps the table to the key map 
 */
void  set_key_pins_map( unsigned int*  row, unsigned int* col, int table[COLNUM][ROWNUM] )
{
	for(int i = 0 ; i < COLNUM ; i++)	{
		col_pins[i] = col[i]; // set col
	}
	for(int j = 0 ; j < ROWNUM ; j++) 	{
		row_pins[j] = row[j]; 	// set row
	}
	//Mapping table to its key map 
	for(int i = 0 ; i < COLNUM ; i++) {
		for(int j = 0 ; j < ROWNUM ; j++) {
			key_map[i][j] = table[i][j];
		}
	}
}

/** @fn getkey
 * @brief This function returns the corresponding value in the keymap
 * @details This function returns the value from key map 
 * @param col_row The input to return the correspoding mapped value in key_map table 
 * @return Returns the corresponding value based on the input from the key_map table
 */
int get_key(int col_row)
{
	return key_map[col_row>>16][0xFFFF & col_row];
}

/** @fn get_col_row
 * @brief This function returns the column-row
 * @details Identifies the pressed key by making the corresponding coloumn
   low and reading the row values     
 * @return -1 - No keys pressed ; else - Returns the key pressed by the Keypad
 */
int get_col_row(void)
{
	int col_row = 0;
	unsigned long read_value = 0;
	unsigned long temp_read_data = 0;

	//Masking Data Reg 
	temp_read_data = read_word(GPIO_DATA_REG);
	write_word(GPIO_DATA_REG,( temp_read_data | (0xF << OUTPINS_OFFSET) ) );

	for(int i= 0 ; i < COLNUM ; i++)
	{
		temp_read_data = read_word(GPIO_DATA_REG);
		write_word(GPIO_DATA_REG, ((0xF << OUTPINS_OFFSET) & (~(0x1 <<i))));

		for(int j = 0 ; j < ROWNUM ; j++) {
			read_value =  read_word(GPIO_DATA_REG);
			log_debug("\n The read value is %lx; row_pins: %x", (read_value), row_pins[j]);
			if( ( read_value & (1 << row_pins[j] ) ) == 0 ) {
				log_debug("\n The read value is %lx; [i:%d; j:%d] row_pins: %d; key_map=%d", read_value, i, j, row_pins[j], key_map[i][j]);
				return key_map[i][j];
			}
		}

		temp_read_data = read_word(GPIO_DATA_REG);
		write_word(GPIO_DATA_REG,(temp_read_data | (0xF << OUTPINS_OFFSET)));
	}
	return -1;
}

/** @fn gpio_init
 * @brief This function initializes the GPIOs, Keypad and PIR Sensor. 
 * @details Initializing the GPIOs used by the sensors and keypad to its mode.    
 */
void gpio_init()
{
	set_key_pins_map(row, col,keyTable);

	//Configuring the Pin mode for pins for keypad
	unsigned long read_data = 0;

	read_data = read_word(GPIO_DIRECTION_CNTRL_REG);
	write_word (GPIO_DIRECTION_CNTRL_REG, ( read_data | (OUTPINS ) ) );

	//Configuring the Pin mode for GPIO pins for other peripherals
	read_data = read_word(GPIO_DIRECTION_CNTRL_REG);
	//Configure direction of GPIO pins (1 ---> Output, 0 --> input)
	write_word(GPIO_DIRECTION_CNTRL_REG, ( read_data | GPIO_OUTPINS ) );

	//All the LED initially set to LOW
	write_gpio(LED_P1,LOW);
	write_gpio(LED_P2,LOW);
	write_gpio(LED_P3,LOW);
	write_gpio(LED_P4,LOW);
	write_gpio(BUZZER,LOW);
}

/** @fn main
 * @brief This is the first function called when the program is executed. 
 * @details Initializes the necessary devices and pins and then runs the main intruder detection program   
 * @return 0 
 */
int main()
{
	//Clearing the Data Register
	unsigned long read_data = 0;
	int prev_col_row = 0;

	printf("\n\n\tIntrusion Detection System");
	gpio_init(); //Intialization of gpio 

	printf("\nInitialising...");

	while(1)
	{
		read_data = read_word(GPIO_DATA_REG);

		if(POWER_SWITCH & read_data) {
			if(first_flag){
				//First time giving the sensor to calibrate 
				delay(30);
				printf("\nIntruder System activated");
				first_flag=0;
				alarm_counter_flag =1;
			}
			//Intruder System - ON
			write_gpio(LED_P2,HIGH);
			//Deactivation of Alarm Switch
		
			if(DEACTIVATE_SWITCH & read_data) {
				deactivate_flag =1;
			}
			else {
				deactivate_flag =0;
			}

			int temp_read_PIR  = pir_read_state(read_data);
			int temp_read_panic = panic_button_read_state(read_data);
	
			alarm_present_state= ((temp_read_PIR > 0) || (temp_read_panic == 1) || alarm_flag == 1 ) && (deactivate_flag == 0);

			if(alarm_present_state == 1 && alarm_previous_state == 0) {
				log_debug("\nAlarm state- %x   %x",alarm_previous_state,alarm_present_state);

				if(temp_read_PIR == 1)
					printf("\nHuman Detection\nDeactivate to stop Alarm");
				else if(temp_read_panic == 1)
					printf("\nPanic Button\nDeactivate to stop Alarm");
				//else {printf("\nDeactivate the Alarm to stop...");}

				write_gpio(LED_P1,HIGH);
				buzzer_tone();
				alarm_flag = 1;
				alarm_previous_state = alarm_present_state;

			}
			else if(alarm_present_state == 1 && alarm_previous_state == 1) {
				log_debug("\n%x   %x",alarm_previous_state,alarm_present_state);
				write_gpio(LED_P1,HIGH);
				buzzer_tone();
				alarm_flag =1;
				alarm_previous_state = alarm_present_state;
				alarm_counter++;
				//printf("\n\t\t\t%x",alarm_counter);

				if(alarm_counter == ALARM_TIMER & ( alarm_counter_flag == 1 ))
				{
					write_gpio(LED_P3,HIGH);
					printf("\nContacting Police for Assistance");

					alarm_counter_flag = 0;
					alarm_counter = 0;
				}
				else if(alarm_counter == ALARM_TIMER) {
					//Making Sure the alarm counter does not overflow
					alarm_counter = 0;
				}

			}
			else if(alarm_present_state == 0 && alarm_previous_state == 1)
			{
				alarm_flag = 0;
				write_gpio(LED_P1,LOW);


				write_gpio(LED_P3,LOW);
				alarm_counter_flag = 1;
				alarm_counter= 0;

				printf("\nAlarm deactivated successfully");
				alarm_previous_state = alarm_present_state;
			}
			else {
				//Flags
				alarm_flag = 0;
				alarm_counter= 0;
				alarm_counter_flag = 1;

				//write_gpio(LED_P3,LOW);
				write_gpio(LED_P1,LOW);
				alarm_previous_state = alarm_present_state ;
			}

			col_row = get_col_row();
			//KeyPad Sensor 
			if( col_row != -1 ) {
				printf("\nThe %x Key Pressed", col_row);

				if( col_row == key_map[3][3]) {
					printf("\nEnter Password-");
				}
				delay(1);
			}
		}
		else {
			//Intruder System Off
			write_gpio(LED_P2,LOW);
			first_flag = 1;
		}
		//end of infinite while loop
	}
	return 0;
}
