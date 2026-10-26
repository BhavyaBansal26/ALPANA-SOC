/***************************************************************************
 * Project           		: shakti devt board
 * Name of the file	     	: csr_test.c
 * Brief Description of file     : Demonstrates the read/write csr registers.
 * Name of Author    	        : Sathya Narayanan N
 * Email ID                      : sathya281@gmail.com

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
  @file   csr_test.c
  @brief  An example program to demonstrate csr read and write.
  @detail CSR registers are read and written here. This examples
  also illustrates the use of inline assembly.
 */

#include "uart.h"//Includes the definitions of uart communication protocol//
#include <stdio.h>

/** @fn void main()
 * @brief prints csr regiter values.
 */
void main()
{
	register uint32_t retval;

	asm volatile("li      t0, 8\t\n"
		     "csrrs   zero, mstatus, t0\t\n"
		    );

	asm volatile( "csrr %[retval], mstatus\n"
		      :
		      [retval]
		      "=r"
		      (retval)
		    );

	printf("mstatus = %x\n", retval);

	asm volatile("li      t0, 128\t\n"
		     "csrrs   zero, mie, t0\t\n"
		    );

	asm volatile( "csrr %[retval], mie\n"
		      :
		      [retval]
		      "=r"
		      (retval)
		    );

	printf("mie = %x\n", retval);
}
