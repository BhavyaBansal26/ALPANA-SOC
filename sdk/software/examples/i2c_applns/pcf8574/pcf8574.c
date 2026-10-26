/*************************************************************************
 * Project           	       : shakti devt board
 * Name of the file	       : pcf8574.c
 * Brief Description of file   : Contains the driver routines to configure pcf8574.
 * Name of Author              : Kotteeswaran
 * Email ID                    : kottee.1@gmail.com

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
@file   pcf8574.c
@brief  Contains the driver routines to configure pcf8574.
@detail I2C based routines to configure pcf8574.
*/

#include <stdint.h>//Includes the definitions of standard input/output functions//
#include "i2c.h"
#include "log.h"
#include "uart.h"
#include "i2c.h"
#include "log.h"
#include "uart.h"

#define PCF8574_SLAVE_ADDRESS 0x4E
#define I2C i2c_instance[0]
#define PRESCALER_COUNT 0x1F
#define SCLK_COUNT 0x91
#define PCF8574_DEC_TO_HEX(decimal)  ( ( (decimal / 10 ) << 4) | (decimal % 10) )
#define DELAY_VALUE 900
#define PCF8574_REG_OFFSET 0

/** @fn int write_pcf8574_registers(i2c_struct * i2c_instance, unsigned int reg_offset, unsigned int write_value, unsigned long delay)
 * @brief  configures and write the pcf8574 
 * @param i2c instance
 * @param reg_offset
 * @param *write_value
 * @param delay
 * @return write status (Zero on SUCCESS.)
 */
int write_pcf8574_registers(i2c_struct * i2c_instance,unsigned int reg_offset, unsigned int write_value, unsigned long delay)
{
	int i = 0, j = 0,  k = 0, status=0;
	unsigned int temp = 0;
	i2c_send_slave_address(i2c_instance, PCF8574_SLAVE_ADDRESS, I2C_WRITE, delay);
	i2c_write_data(i2c_instance,  ( write_value & 0xff), delay);
	//Stops the I2C transaction to start reading the temperature value.
	i2c_instance->control = I2C_STOP;
	// delay_loop(800, 800);
	return 0;
}

/** @fn void main()
 * @brief configures and write the pcf8574 IO expander.
 * @details 
 */
void main()
{
	int timeout;
	unsigned int tempReadValue = 0;
	unsigned long delay = 1000;

	printf("\nPCF8574 IO Expander Test");

	i2c_init();

	if(config_i2c(I2C, PRESCALER_COUNT, SCLK_COUNT))
	{
		log_error("\tSomething Wrong In Initialization\n");
		return -1;
	}
		else
		log_info("\tIntilization Happened Fine\n");
		printf("\n Write he scomplete");
	while(1)
	{
		printf("\n Writing 0");
		write_pcf8574_registers(I2C, 0x00, 0x00, delay);
		delay_loop(2000, 5000);
		printf("\n Writing 1");
		write_pcf8574_registers(I2C, 0x00, 0xFF, delay);
		delay_loop(2000, 5000);
	}
}




