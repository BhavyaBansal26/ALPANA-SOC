/***************************************************************************
* Project                               : shakti devt board
* Name of the file                      : spi.c
* Brief Description of file             : write data to flash by spi 
* Name of Author                        : Kaustubh Ghormade, Kotteswaran
* Email ID                              : kaustubh4347@gmail.com

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
@file spi.c
@brief  write data to flash by spi
@detail Contains driver codes to read and write flash using SPI interface.
*/

#include <stdint.h>
#include "spi.h"
#include "uart.h"
#include "utils.h"

#define DELAY1 1000//Determines the duration of delay1//
#define DELAY2 2000//Determines the duration of delay2//

/** @fn static void flash_read_locations(uint32_t read_address, uint16_t length)
 * @brief Reads the flash memory contents starting from read_address.
          Totally length number of bytes will be read.
 * @param unsigned int (32-bit).
 * @param unsigned int (16-bit).           
 */
static void flash_read_locations(uint32_t read_address, uint16_t length)
{
	for(int i = 0; i < length; ++i)
	{
		int read_value= flash_read(read_address);
		printf("\nReading from adddress %x and data \
			   %x\n",read_address,read_value);
		read_address=read_address+4;
	}
}

/** @fn void main()
 * @brief Configures the SPI flash and writes into a flash location.
 * @details Configures the SPI flash, Confirms the flash device id,erases a sector
 *           and then write into a flash location and prints the value.
 */
void main()
{
	int write_address = 0x00b00004;  // read/write from/to this address
	int read_address  = 0x00b00004;  // read/write from/to this address
	int data = 0xDEADBEEF; //32 bits of data can be written at a time
	uint16_t length = 11;

	configure_spi(SPI0_OFFSET);
	spi_init();

	printf("SPI init done\n");

	flash_device_id(); 

	waitfor(200);

	printf("\n Before Erase");
	flash_read_locations(read_address, length);
	flash_write_enable();
	flash_erase(0x00b00000); //erases an entire sector
	flash_status_register_read();
	printf("\n After Erase");
	flash_read_locations(read_address, length);

	//flash write
	flash_write( write_address, 0x12345678);
	flash_write( write_address + 0x04, 0xaaaaaaaa);
	flash_write( write_address + 0x08, 0x55555555);
	flash_write( write_address + 0x0C, 0xAAAA5555);
	flash_write( write_address + 0x10, 0x5555AAAA);
	flash_write( write_address + 0x14, 0xaaa5aaa5);
	flash_write( write_address + 0x18, 0x555A555A);
	flash_write( write_address + 0x1c, 0xaa55aa55);
	flash_write( write_address + 0x20, 0x55aa55aa);
	flash_write( write_address + 0x24, 0x5a5a5a5a);
	flash_write( write_address + 0x28, 0xa5a5a5a5);

	printf("\nFlash write done on address %x and data %x \n", 
	       write_address, data);

	printf("\n After Write");
	flash_read_locations(read_address, length);
	asm volatile ("ebreak");
}
