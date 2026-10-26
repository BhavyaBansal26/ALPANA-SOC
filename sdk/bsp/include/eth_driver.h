/***************************************************************************
* Project           	     : shakti devt board
* Name of the file	     : ETH_driver.h
* Brief Description of file  : Header file for ETH driver.
* Name of Author    	     : 
* Email ID                   : sathya281@gmail.com

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
 * @file plic_driver.h
 * @brief  Header file for plic driver.c
 * @detail This file contains the definitions for plic driver. The memory map
 * for the plic module is also defined here.
 */

#ifndef ETH_DRIVER_H
#define ETH_DRIVER_H
#include "platform.h"
#include "traps.h"
#include "eth_driver.h"
#include "eth_macros.h"

#define INLINE inline
#define __LITTLE_ENDIAN__ 1


/*
 * guarded against xbasic_types.h.
 */
typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
/** @}*/
#define __XUINT64__
typedef struct
{
	u32 Upper;
	u32 Lower;
} Xuint64;

typedef char char8;
typedef int8_t s8;
typedef int16_t s16;
typedef int32_t s32;
typedef int64_t s64;
typedef uint64_t u64;
typedef int sint32;

typedef intptr_t INTPTR;
typedef uintptr_t UINTPTR;
//typedef ptrdiff_t PTRDIFF;

/*****************************************************************************
 *
 * EmacLite defines .
 * DeviceID starts at 240
 */
#define XPAR_XEMACLITE_NUM_INSTANCES    1
#define XPAR_EMACLITE_0_DEVICE_ID       240
/* EK Change */
#define XPAR_EMACLITE_0_BASEADDR      0x00044000 //0
#define XPAR_EMACLITE_0_TX_PING_PONG    1
#define XPAR_EMACLITE_0_RX_PING_PONG    1
#define XPAR_EMACLITE_0_INCLUDE_MDIO 1
#define XPAR_EMACLITE_0_INCLUDE_INTERNAL_LOOPBACK 1


#define TRUE		1U
#define FALSE		0U

#define NULL		0U
#define SUCCESS 0
#define FAILURE -1
#define DEVICE_BUSY                 21L	/*!< Device is busy */
#define NO_CALLBACK 18L	/*!< A callback has not yet been
						   registered */
#define NO_DATA                     13L	/*!< There was no data available */
#define XST_LOOPBACK_ERROR              17L	/*!< A loopback test failed */
#define XST_NO_CALLBACK                 18L	/*!< A callback has not yet been
						   registered */
#define XST_NO_FEATURE                  19L	/*!< Device is not configured with
						   the requested feature */
#define XST_NOT_INTERRUPT               20L	/*!< Device is not configured for
						   interrupt mode operation */

#define XIL_COMPONENT_IS_READY     0x11111111U  /**< In device drivers, This macro will be
                                                 assigend to "IsReady" member of driver
												 instance to indicate that driver
												 instance is initialized and ready to use. */
#define XIL_COMPONENT_IS_STARTED   0x22222222U  /**< In device drivers, This macro will be assigend to
                                                 "IsStarted" member of driver instance
												 to indicate that driver instance is
												 started and it can be enabled. */
#define XST_SUCCESS SUCCESS
#define XST_FAILURE FAILURE
#define xil_printf log_debug

/************************** Constant Definitions ****************************/

#define PHY_REG0_OFFSET		0 /* Register 0 of PHY device */
#define PHY_REG1_OFFSET 	1 /* Register 1 of PHY device */

#define PHY_REG0_RESET_MASK	0x8000  /* Reset Phy device */
#define PHY_REG0_LOOPBACK_MASK	0x4000  /* Loopback Enable in Phy */
#define PHY_REG0_SPD_100_MASK	0x2000  /* Speed of 100Mbps for Phy */

#define PHY_REG1_DETECT_MASK	0x1808	/* Mask to detect PHY device */

#define EMACLITE_PHY_DELAY_SEC	4	/* Amount of time to delay waiting on
					 * PHY to reset.
					 */


/**
 * This typedef contains configuration information for a device.
 */
typedef struct {
	u16 DeviceId;	 /**< Unique ID  of device */
	UINTPTR BaseAddress; /**< Device base address */
	u8 TxPingPong;	 /**< 1 if TX Pong buffer configured, 0 otherwise */
	u8 RxPingPong;	 /**< 1 if RX Pong buffer configured, 0 otherwise */
	u8 MdioInclude;  /**< 1 if MDIO is enabled, 0 otherwise */
	u8 Loopback;     /**< 1 if internal loopback is enabled, 0 otherwise */
} XEmacLite_Config;

/*
 * Callback when data is sent or received .
 * @param 	CallBackRef is a callback reference passed in by the upper layer
 *		when setting the callback functions, and passed back to the
 *		upper layer when the callback is invoked.
 */
typedef void (*XEmacLite_Handler) (void *CallBackRef);

/**
 * The XEmacLite driver instance data. The user is required to allocate a
 * variable of this type for every EmacLite device in the system. A pointer
 * to a variable of this type is then passed to the driver API functions.
 */
typedef struct {
	XEmacLite_Config EmacLiteConfig; /* Device configuration */
	u32 IsReady;			 /* Device is initialized and ready */

	u32 NextTxBufferToUse;		 /* Next TX buffer to write to */
	u32 NextRxBufferToUse;		 /* Next RX buffer to read from */

	/*
	 * Callbacks
	 */
	XEmacLite_Handler RecvHandler;
	void *RecvRef;
	XEmacLite_Handler SendHandler;
	void *SendRef;

} XEmacLite;

#ifndef ETH_VARI
extern volatile u32 RecvFrameLength;
extern UINTPTR TxBuffPtr;
extern UINTPTR RxBuffPtr;
extern volatile int FramesTx;
extern volatile int FramesRx;
extern volatile int TxCount;
extern volatile int RxCount;
extern volatile int Padding;	/* For 1588 Packets we need to pad 8 bytes time stamp value */
#endif



/****************************************************************************/
/**
*
* Return the next expected Transmit Buffer's address.
*
* @param	InstancePtr is the pointer to the instance of the driver to
*		be worked on
*
* @note		C-Style signature:
*		u32 XEmacLite_NextTransmitAddr(XEmacLite *InstancePtr);
*
* This macro returns the address of the next transmit buffer to put data into.
* This is used to determine the destination of the next transmit data frame.
*
*****************************************************************************/
#define XEmacLite_NextTransmitAddr(InstancePtr) 			\
	((InstancePtr)->EmacLiteConfig.BaseAddress + 			\
		(InstancePtr)->NextTxBufferToUse) + XEL_TXBUFF_OFFSET

/****************************************************************************/
/**
*
* Return the next expected Receive Buffer's address.
*
* @param	InstancePtr is the pointer to the instance of the driver to
*		be worked on
*
* @note		C-Style signature:
*		u32 XEmacLite_NextReceiveAddr(XEmacLite *InstancePtr);
*
* This macro returns the address of the next receive buffer to read data from.
* This is the expected receive buffer address if the driver is in sync.
*
*****************************************************************************/
#define XEmacLite_NextReceiveAddr(InstancePtr)				\
	((InstancePtr)->EmacLiteConfig.BaseAddress + 			\
	(InstancePtr)->NextRxBufferToUse)

/*****************************************************************************/
/**
*
* This macro determines if the device is currently configured for MDIO.
*
* @param	InstancePtr is the pointer to the instance of the
*		EmacLite driver.
*
* @return
* 		- TRUE if the device is configured for MDIO.
*		- FALSE if the device is NOT configured for MDIO.
*
* @note		C-Style signature:
*		int XEmacLite_IsMdioConfigured(XEmacLite *InstancePtr)
*
******************************************************************************/
#define XEmacLite_IsMdioConfigured(InstancePtr) 			\
	((InstancePtr)->EmacLiteConfig.MdioInclude == 1)

/*****************************************************************************/
/**
*
* This macro determines if the device is currently configured for internal
* loopback.
*
* @param	InstancePtr is the pointer to the instance of the
*		EmacLite driver.
*
* @return
* 		- TRUE if the device is configured for internal loopback.
*		- FALSE if the device is NOT configured for internal loopback.
*
* @note		C-Style signature:
*		int XEmacLite_IsLoopbackConfigured(XEmacLite *InstancePtr)
*
******************************************************************************/
#define XEmacLite_IsLoopbackConfigured(InstancePtr) 			\
            ((InstancePtr)->EmacLiteConfig.Loopback == 1)

/************************** Variable Definitions *****************************/


void XEmacLite_AlignedWrite(void *SrcPtr, UINTPTR *DestPtr, unsigned ByteCount);
void XEmacLite_AlignedRead(UINTPTR *SrcPtr, void *DestPtr, unsigned ByteCount);

extern XEmacLite_Config XEmacLite_ConfigTable;
extern int XEmacLite_CfgInitialize(XEmacLite *InstancePtr,
				XEmacLite_Config *EmacLiteConfigPtr,
				UINTPTR EffectiveAddr);


#endif
