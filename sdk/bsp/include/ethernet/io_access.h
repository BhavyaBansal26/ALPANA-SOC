/**************************************************************************
 * Project           	         : shakti devt board
 * Name of the file	         : io_access.h
 * Brief Description of file     : header file for accessing io addresses.
 * Name of Author    	         : Kotteeswaran 
 * Email ID                      : kottee.1@gmail.com
 
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
 * @file io_access.h
 * @brief  Header file for accessing io lines
 * @detail this is the header file for accessing all io lines mapped to shakti core.
 */

#ifndef IO_ACCESS_H           /* prevent circular inclusions */
#define IO_ACCESS_H           /* by using protection macros */

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/

#include "xil_types.h"
#include "log_debug.h"
#include "xstatus.h"

#if defined (__MICROBLAZE__)
#include "mb_interface.h"
#else
#include "xpseudo_asm.h"
#endif

/************************** Function Prototypes ******************************/
u16 Xil_EndianSwap16(u16 Data);
u32 Xil_EndianSwap32(u32 Data);
#ifdef ENABLE_SAFETY
extern u32 XStl_RegUpdate(u32 RegAddr, u32 RegVal);
#endif

/***************** Macros (Inline Functions) Definitions *********************/
#if defined __GNUC__
#if defined (__MICROBLAZE__)
#  define INST_SYNC		mbar(0)
#  define DATA_SYNC		mbar(1)
# else
#  define SYNCHRONIZE_IO	dmb()
#  define INST_SYNC		isb()
#  define DATA_SYNC		dsb()
# endif
#else
# define SYNCHRONIZE_IO
# define INST_SYNC
# define DATA_SYNC
# define INST_SYNC
# define DATA_SYNC
#endif

#if defined (__GNUC__) || defined (__ICCARM__) || defined (__MICROBLAZE__)
#define INLINE inline
#else
#define INLINE __inline
#endif

/*****************************************************************************/
/**
*
* @brief    Performs an input operation for a memory location by reading
*           from the specified address and returning the 8 bit Value read from
*            that address.
*
* @param	Addr: contains the address to perform the input operation
*
* @return	The 8 bit Value read from the specified input address.

*
******************************************************************************/
static INLINE uint8_t get_io8(UINTPTR Addr)
{
	return *(volatile uint8_t *) Addr;
}

/*****************************************************************************/
/**
*
* @brief    Performs an input operation for a memory location by reading from
*           the specified address and returning the 16 bit Value read from that
*           address.
*
* @param	Addr: contains the address to perform the input operation
*
* @return	The 16 bit Value read from the specified input address.
*
******************************************************************************/
static INLINE uin16_t get_io16(UINTPTR Addr)
{
	return *(volatile uin16_t *) Addr;
}

/*****************************************************************************/
/**
*
* @brief    Performs an input operation for a memory location by
*           reading from the specified address and returning the 32 bit Value
*           read  from that address.
*
* @param	Addr: contains the address to perform the input operation
*
* @return	The 32 bit Value read from the specified input address.
*
******************************************************************************/
static INLINE uint32_t get_io32(UINTPTR Addr)
{
	return *(volatile uint32_t *) Addr;
}

/*****************************************************************************/
/**
*
* @brief     Performs an input operation for a memory location by reading the
*            64 bit Value read  from that address.
*
*
* @param	Addr: contains the address to perform the input operation
*
* @return	The 64 bit Value read from the specified input address.
*
******************************************************************************/
static INLINE uint64_t get_io64(UINTPTR Addr)
{
	return *(volatile uint64_t *) Addr;
}

/*****************************************************************************/
/**
*
* @brief    Performs an output operation for an memory location by
*           writing the 8 bit Value to the the specified address.
*
* @param	Addr: contains the address to perform the output operation
* @param	Value: contains the 8 bit Value to be written at the specified
*           address.
*
* @return	None.
*
******************************************************************************/
static INLINE void set_io8(UINTPTR Addr, uint8_t Value)
{
	volatile uint8_t *LocalAddr = (volatile uint8_t *)Addr;
	*LocalAddr = Value;
}

/*****************************************************************************/
/**
*
* @brief    Performs an output operation for a memory location by writing the
*            16 bit Value to the the specified address.
*
* @param	Addr contains the address to perform the output operation
* @param	Value contains the Value to be written at the specified address.
*
* @return	None.
*
******************************************************************************/
static INLINE void set_io16(UINTPTR Addr, uint16_t Value)
{
	volatile uint16_t *LocalAddr = (volatile uint16_t *)Addr;
	*LocalAddr = Value;
}

/*****************************************************************************/
/**
*
* @brief    Performs an output operation for a memory location by writing the
*           32 bit Value to the the specified address.
*
* @param	Addr contains the address to perform the output operation
* @param	Value contains the 32 bit Value to be written at the specified
*           address.
*
* @return	None.
*
******************************************************************************/
static INLINE void set_io32(UINTPTR Addr, uint32_t Value)
{
	volatile uint32_t *LocalAddr = (volatile uint32_t *)Addr;
	*LocalAddr = Value;
}

/*****************************************************************************/
/**
*
* @brief    Performs an output operation for a memory location by writing the
*           64 bit Value to the the specified address.
*
* @param	Addr contains the address to perform the output operation
* @param	Value contains 64 bit Value to be written at the specified address.
*
* @return	None.
*
******************************************************************************/
static INLINE void set_io64(UINTPTR Addr, uint64_t Value)
{
	volatile uint64_t *LocalAddr = (volatile uint64_t *)Addr;
	*LocalAddr = Value;
}

/*****************************************************************************/
/**
 *
 * @brief	Performs an output operation for a memory location by writing the
 *       	32 bit Value to the the specified address and then reading it
 *       	back to verify the value written in the register.
 *
 * @param	Addr contains the address to perform the output operation
 * @param	Value contains 32 bit Value to be written at the specified address
 *
 * @return	Returns Status
 *        	- SUCCESS on success
 *        	- FAILURE on failure
 *
 *****************************************************************************/
static INLINE u32 Xil_SecureOut32(UINTPTR Addr, u32 Value)
{
	u32 ReadReg;
	u32 ReadRegTemp;
	u32 Status = (u32)FAILURE;

	Xil_Out32(Addr, Value);

	ReadReg = Xil_In32(Addr);
	ReadRegTemp = Xil_In32(Addr);

	if( (ReadReg == Value) && (ReadRegTemp == Value) ) {
		Status = (u32)SUCCESS;
	}

	return Status;
}

#if defined (__MICROBLAZE__)
#ifdef __LITTLE_ENDIAN__
# define Xil_In16LE	get_io16
# define Xil_In32LE	get_io32
# define Xil_Out16LE	set_io16
# define Xil_Out32LE	set_io32
# define Xil_Htons	Xil_EndianSwap16
# define Xil_Htonl	Xil_EndianSwap32
# define Xil_Ntohs	Xil_EndianSwap16
# define Xil_Ntohl	Xil_EndianSwap32
# else
# define Xil_In16BE	get_io16
# define Xil_In32BE	get_io32
# define Xil_Out16BE	set_io16
# define Xil_Out32BE	set_io32
# define Xil_Htons(Data) (Data)
# define Xil_Htonl(Data) (Data)
# define Xil_Ntohs(Data) (Data)
# define Xil_Ntohl(Data) (Data)
#endif
#else
# define Xil_In16LE	get_io16
# define Xil_In32LE	get_io32
# define Xil_Out16LE	set_io16
# define Xil_Out32LE	set_io32
# define Xil_Htons	Xil_EndianSwap16
# define Xil_Htonl	Xil_EndianSwap32
# define Xil_Ntohs	Xil_EndianSwap16
# define Xil_Ntohl	Xil_EndianSwap32
#endif

#if defined (__MICROBLAZE__)
#ifdef __LITTLE_ENDIAN__
static INLINE u16 Xil_In16BE(UINTPTR Addr)
#else
static INLINE u16 Xil_In16LE(UINTPTR Addr)
#endif
#else
static INLINE u16 Xil_In16BE(UINTPTR Addr)
#endif
{
	u16 value = Xil_In16(Addr);
	return Xil_EndianSwap16(value);
}

#if defined (__MICROBLAZE__)
#ifdef __LITTLE_ENDIAN__
static INLINE u32 Xil_In32BE(UINTPTR Addr)
#else
static INLINE u32 Xil_In32LE(UINTPTR Addr)
#endif
#else
static INLINE u32 Xil_In32BE(UINTPTR Addr)
#endif
{
	u32 value = Xil_In32(Addr);
	return Xil_EndianSwap32(value);
}

#if defined (__MICROBLAZE__)
#ifdef __LITTLE_ENDIAN__
static INLINE void Xil_Out16BE(UINTPTR Addr, u16 Value)
#else
static INLINE void Xil_Out16LE(UINTPTR Addr, u16 Value)
#endif
#else
static INLINE void Xil_Out16BE(UINTPTR Addr, u16 Value)
#endif
{
	Value = Xil_EndianSwap16(Value);
	Xil_Out16(Addr, Value);
}

#if defined (__MICROBLAZE__)
#ifdef __LITTLE_ENDIAN__
static INLINE void Xil_Out32BE(UINTPTR Addr, u32 Value)
#else
static INLINE void Xil_Out32LE(UINTPTR Addr, u32 Value)
#endif
#else
static INLINE void Xil_Out32BE(UINTPTR Addr, u32 Value)
#endif
{
	Value = Xil_EndianSwap32(Value);
	Xil_Out32(Addr, Value);
}

#ifdef __cplusplus
}
#endif

#endif /* end of protection macro */
/**
* @} End of "addtogroup common_io_interfacing_apis".
*/

