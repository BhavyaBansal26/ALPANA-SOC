/***************************************************************************
 * Project           		: shakti devt board
 * Name of the file	     	: plic_driver.c
 * Brief Description of file    : source file for plic.
 * Name of Author    	        : Sathya Narayanan N
 * Email ID                     : sathya281@gmail.com

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
  @file plic_driver.c
  @brief source file for plic driver
  @detail This file contains the driver code for plic device. The functions to
  setup each plic registers, isr routine and plic interrupt handler are here.
 */
#define ETH_VARI 1

#include "eth_driver.h"
#include "plic_driver.h"
#include "platform.h"
#include "log.h"
#include "stddef.h"
#include "gpio.h"
#include "utils.h"

/*
 * Variable used to indicate the length of the received frame.
 */
volatile u32 RecvFrameLength;
UINTPTR TxBuffPtr;
UINTPTR RxBuffPtr;
volatile int FramesTx;
volatile int FramesRx;
volatile int TxCount;
volatile int RxCount;
volatile int Padding;	/* For 1588 Packets we need to pad 8 bytes time stamp value */

/*
 * Set up valid local MAC addresses. This loop back test uses the LocalAddress
 * both as a source and destination MAC address.
 */

XEmacLite EmacLiteInstance;	/* Instance of the EmacLite */

/*
 * Buffers used for Transmission and Reception of Packets. These are declared
 * as global so that they are not a part of the stack.
 */
u8 TxFrame[XEL_MAX_FRAME_SIZE];
u8 RxFrame[XEL_MAX_FRAME_SIZE];

volatile u32 RecvFrameLength;	/* Indicates the length of the Received packet
				 */
volatile int TransmitComplete;	/* Flag to indicate that the Transmission
				 * is complete
				 */


void StubHandler(void *CallBackRef);


/**
 * This table contains configuration information for each EmacLite device
 * in the system.
 */
XEmacLite_Config XEmacLite_ConfigTable = {
	{
	 XPAR_EMACLITE_0_DEVICE_ID,	/* Unique ID of device */
	 XPAR_EMACLITE_0_BASEADDR,	/* Device base address */
	 XPAR_EMACLITE_0_TX_PING_PONG,	/* Include TX Ping Pong buffers */
	 XPAR_EMACLITE_0_RX_PING_PONG,	/* Include RX Ping Pong buffers */
	 XPAR_EMACLITE_0_INCLUDE_MDIO,	/* Include MDIO support */
	 XPAR_EMACLITE_0_INCLUDE_INTERNAL_LOOPBACK
/* Include Internal  loop back support */
	 }
};



/************************** Function Prototypes ******************************/

static u16 XEmacLite_GetReceiveDataLength(UINTPTR BaseAddress);

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
static INLINE u8 Xil_In8(UINTPTR Addr)
{
	return *(volatile u8 *) Addr;
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
static INLINE u16 Xil_In16(UINTPTR Addr)
{
	return *(volatile u16 *) Addr;
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
static INLINE u32 Xil_In32(UINTPTR Addr)
{
	return *(volatile u32 *) Addr;
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
static INLINE u64 Xil_In64(UINTPTR Addr)
{
	return *(volatile u64 *) Addr;
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
static INLINE void Xil_Out8(UINTPTR Addr, u8 Value)
{
	volatile u8 *LocalAddr = (volatile u8 *)Addr;
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
static INLINE void Xil_Out16(UINTPTR Addr, u16 Value)
{
	volatile u16 *LocalAddr = (volatile u16 *)Addr;
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
static INLINE void Xil_Out32(UINTPTR Addr, u32 Value)
{
	volatile u32 *LocalAddr = (volatile u32 *)Addr;
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
static INLINE void Xil_Out64(UINTPTR Addr, u64 Value)
{
	volatile u64 *LocalAddr = (volatile u64 *)Addr;
	*LocalAddr = Value;
}

/*****************************************************************************/
/**
*
* @brief    Perform a 16-bit endian conversion.
*
* @param	Data: 16 bit value to be converted
*
* @return	16 bit Data with converted endianness
*
******************************************************************************/
u16 Xil_EndianSwap16(u16 Data)
{
	return (u16) (((Data & 0xFF00U) >> 8U) | ((Data & 0x00FFU) << 8U));
}

/*****************************************************************************/
/**
*
* @brief    Perform a 32-bit endian conversion.
*
* @param	Data: 32 bit value to be converted
*
* @return	32 bit data with converted endianness
*
******************************************************************************/
u32 Xil_EndianSwap32(u32 Data)
{
	u16 LoWord;
	u16 HiWord;

	/* get each of the half words from the 32 bit word */

	LoWord = (u16) (Data & 0x0000FFFFU);
	HiWord = (u16) ((Data & 0xFFFF0000U) >> 16U);

	/* byte swap each of the 16 bit half words */

	LoWord = (((LoWord & 0xFF00U) >> 8U) | ((LoWord & 0x00FFU) << 8U));
	HiWord = (((HiWord & 0xFF00U) >> 8U) | ((HiWord & 0x00FFU) << 8U));

	/* swap the half words before returning the value */

	return ((((u32)LoWord) << (u32)16U) | (u32)HiWord);
}


/************************** Variable Definitions *****************************/



u16 Xil_Htons(u16 Data)
{
	return (u16) (((Data & 0xFF00U) >> 8U) | ((Data & 0x00FFU) << 8U));
}

u16 Xil_Ntohs(u16 Data)
{
	return (u16) (((Data & 0xFF00U) >> 8U) | ((Data & 0x00FFU) << 8U));
}


#if 0
#define Xil_In16LE	Xil_In16
#define Xil_In32LE	Xil_In32
#define Xil_Out16LE	Xil_Out16
#define Xil_Out32LE	Xil_Out32
#define Xil_Htons	Xil_EndianSwap16
#define Xil_Htonl	Xil_EndianSwap32
#define Xil_Ntohs	Xil_EndianSwap16
#define Xil_Ntohl	Xil_EndianSwap32
#endif


/****************************************************************************/
/**
*
* Read from the specified EmacLite device register.
*
* @param	BaseAddress contains the base address of the device.
* @param	RegOffset contains the offset from the 1st register of the
*		device to select the specific register.
*
* @return	The value read from the register.
*
* @note		C-Style signature:
*		u32 XEmacLite_ReadReg(u32 BaseAddress, u32 RegOffset);
*
******************************************************************************/
#define XEmacLite_ReadReg(BaseAddress, RegOffset) \
	XEmacLite_In32((BaseAddress) + (RegOffset))

/***************************************************************************/
/**
*
* Write to the specified EmacLite device register.
*
* @param	BaseAddress contains the base address of the device.
* @param	RegOffset contains the offset from the 1st register of the
*		device to select the specific register.
* @param	RegisterValue is the value to be written to the register.
*
* @return	None.
*
* @note		C-Style signature:
*		void XEmacLite_WriteReg(u32 BaseAddress, u32 RegOffset,
*					u32 RegisterValue);
******************************************************************************/
#define XEmacLite_WriteReg(BaseAddress, RegOffset, RegisterValue) \
	XEmacLite_Out32((BaseAddress) + (RegOffset), (RegisterValue))


/****************************************************************************/
/**
*
* Get the Tx Status Register Contents.
*
* @param	BaseAddress is the base address of the device
*
* @return	The contents of the Tx Status Register.
*
* @note		C-Style signature:
* 		u32 XEmacLite_GetTxStatus(u32 BaseAddress)
*
*****************************************************************************/
#define XEmacLite_GetTxStatus(BaseAddress)			\
	(XEmacLite_ReadReg((BaseAddress), XEL_TSR_OFFSET))


/****************************************************************************/
/**
*
* Set the Tx Status Register Contents.
*
* @param	BaseAddress is the base address of the device
* @param	Data is the value to be written to the Register.
*
* @return	None.
*
* @note		C-Style signature:
* 		u32 XEmacLite_SetTxStatus(u32 BaseAddress, u32 Data)
*
*****************************************************************************/
#define XEmacLite_SetTxStatus(BaseAddress, Data)			\
	(XEmacLite_WriteReg((BaseAddress), XEL_TSR_OFFSET, (Data)))


/****************************************************************************/
/**
*
* Get the Rx Status Register Contents.
*
* @param	BaseAddress is the base address of the device
*
* @return	The contents of the Rx Status Register.
*
* @note		C-Style signature:
* 		u32 XEmacLite_GetRxStatus(u32 BaseAddress)
*
*****************************************************************************/
#define XEmacLite_GetRxStatus(BaseAddress)			\
	(XEmacLite_ReadReg((BaseAddress), XEL_RSR_OFFSET))


/****************************************************************************/
/**
*
* Set the Rx Status Register Contents.
*
* @param	BaseAddress is the base address of the device
* @param	Data is the value to be written to the Register.
*
* @return	None.
*
* @note		C-Style signature:
* 		u32 XEmacLite_SetRxStatus(u32 BaseAddress, u32 Data)
*
*****************************************************************************/
#define XEmacLite_SetRxStatus(BaseAddress, Data)			\
	(XEmacLite_WriteReg((BaseAddress), XEL_RSR_OFFSET, (Data)))


/****************************************************************************/
/**
*
* Check to see if the transmission is complete.
*
* @param	BaseAddress is the base address of the device
*
* @return	TRUE if it is done, or FALSE if it is not.
*
* @note		C-Style signature:
* 		int XEmacLite_IsTxDone(u32 BaseAddress)
*
*****************************************************************************/
#define XEmacLite_IsTxDone(BaseAddress)			\
		 ((XEmacLite_ReadReg((BaseAddress), XEL_TSR_OFFSET) & 	 \
			 XEL_TSR_XMIT_BUSY_MASK) != XEL_TSR_XMIT_BUSY_MASK)


/****************************************************************************/
/**
*
* Check to see if the receive is empty.
*
* @param	BaseAddress is the base address of the device
*
* @return	TRUE if it is empty, or FALSE if it is not.
*
* @note		C-Style signature:
*		int XEmacLite_IsRxEmpty(u32 BaseAddress)
*
*****************************************************************************/
#define XEmacLite_IsRxEmpty(BaseAddress) \
		  ((XEmacLite_ReadReg((BaseAddress), XEL_RSR_OFFSET) & \
			XEL_RSR_RECV_DONE_MASK) != XEL_RSR_RECV_DONE_MASK)

/************************** Function Prototypes ******************************/

void XEmacLite_SendFrame(UINTPTR BaseAddress, u8 *FramePtr, unsigned ByteCount);
u16 XEmacLite_RecvFrame(UINTPTR BaseAddress, u8 *FramePtr);

/*****************************************************************************/
/**
*
* Send an Ethernet frame. The size is the total frame size, including header.
* This function blocks waiting for the frame to be transmitted.
*
* @param 	BaseAddress is the base address of the device
* @param 	FramePtr is a pointer to frame
* @param 	ByteCount is the size, in bytes, of the frame
*
* @return	None.
*
* @note
*
* This function call is blocking in nature, i.e. it will wait until the
* frame is transmitted. This function can hang and not exit if the
* hardware is not configured properly.
*
* If the ping buffer is the destination of the data, the argument should be
* DeviceAddress + XEL_TXBUFF_OFFSET.
* If the pong buffer is the destination of the data, the argument should be
* DeviceAddress + XEL_TXBUFF_OFFSET + XEL_BUFFER_OFFSET.
* The function does not take the different buffers into consideration.
*
******************************************************************************/
void XEmacLite_SendFrame(UINTPTR BaseAddress, u8 *FramePtr, unsigned ByteCount)
{
	u32 Register;

	/*
	 * Write data to the EmacLite
	 */
	XEmacLite_AlignedWrite(FramePtr, (UINTPTR *) (BaseAddress), ByteCount);

	/*
	 * The frame is in the buffer, now send it
	 */
	XEmacLite_WriteReg(BaseAddress,  XEL_TPLR_OFFSET,
				(ByteCount & (XEL_TPLR_LENGTH_MASK_HI |
				XEL_TPLR_LENGTH_MASK_LO)));


	Register = XEmacLite_GetTxStatus(BaseAddress);
	XEmacLite_SetTxStatus(BaseAddress, Register | XEL_TSR_XMIT_BUSY_MASK);

	/*
	 * Loop on the status waiting for the transmit to be complete.
	 */
	while (!XEmacLite_IsTxDone(BaseAddress));

}


/*****************************************************************************/
/**
*
* Receive a frame. Wait for a frame to arrive.
*
* @param	BaseAddress is the base address of the device
* @param	FramePtr is a pointer to a buffer where the frame will
*		be stored.
*
* @return
*
* The type/length field of the frame received.  When the type/length field
* contains the type , XEL_MAX_FRAME_SIZE bytes will be copied out of the
* buffer and it is up to the higher layers to sort out the frame.
*
* @note
*
* This function call is blocking in nature, i.e. it will wait until a
* frame arrives.
*
* If the ping buffer is the source of the data, the argument should be
* DeviceAddress + XEL_RXBUFF_OFFSET.
* If the pong buffer is the source of the data, the argument should be
* DeviceAddress + XEL_RXBUFF_OFFSET + XEL_BUFFER_OFFSET.
* The function does not take the different buffers into consideration.
*
******************************************************************************/
u16 XEmacLite_RecvFrame(UINTPTR BaseAddress, u8 *FramePtr)
{
	u16 LengthType;
	u16 Length;
	u32 Register;

	/*
	 * Wait for a frame to arrive - this is a blocking call
	 */
	while (XEmacLite_IsRxEmpty(BaseAddress));

	/*
	 * Get the length of the frame that arrived, only 32-bit reads are
	 * allowed LengthType is in the upper half of the 32-bit word.
	 */
	Register = XEmacLite_ReadReg(BaseAddress, XEL_RPLR_OFFSET);
	LengthType = (u16) ((Register >> 16) &
			    (XEL_RPLR_LENGTH_MASK_HI |
			     XEL_RPLR_LENGTH_MASK_LO));

	/*
	 * Check if length is valid
	 */
	if (LengthType > XEL_MAX_FRAME_SIZE) {
		/*
		 * Field contain type, use max frame size and
		 * let user parse it
		 */
		Length = XEL_MAX_FRAME_SIZE;
	}
	else {
		/*
		 * Use the length in the frame, plus the header and trailer
		 */
		Length = LengthType + XEL_HEADER_SIZE + XEL_FCS_SIZE;
	}

	/*
	 * Read each byte from the EmacLite
	 */
	XEmacLite_AlignedRead((UINTPTR *) (BaseAddress + XEL_RXBUFF_OFFSET),
			      FramePtr, Length);

	/*
	 * Acknowledge the frame
	 */
	Register = XEmacLite_GetRxStatus(BaseAddress);
	Register &= ~XEL_RSR_RECV_DONE_MASK;
	XEmacLite_SetRxStatus(BaseAddress, Register);

	return LengthType;
}

/******************************************************************************/
/**
*
* This function aligns the incoming data and writes it out to a 32-bit
* aligned destination address range.
*
* @param	SrcPtr is a pointer to incoming data of any alignment.
* @param	DestPtr is a pointer to outgoing data of 32-bit alignment.
* @param	ByteCount is the number of bytes to write.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void XEmacLite_AlignedWrite(void *SrcPtr, UINTPTR *DestPtr, unsigned ByteCount)
{
	unsigned Index;
	unsigned Length = ByteCount;
	volatile u32 AlignBuffer;
	volatile u32 *To32Ptr;
	u32 *From32Ptr;
	volatile u16 *To16Ptr;
	u16 *From16Ptr;
	volatile u8 *To8Ptr;
	u8 *From8Ptr;

//	To32Ptr = (volatile UINTPTR *)DestPtr;

	if ((((u32) SrcPtr) & 0x00000003) == 0) {

		/*
		 * Word aligned buffer, no correction needed.
		 */
		From32Ptr = (u32 *) SrcPtr;
		To32Ptr = (u32 *)DestPtr;
#if 0
		log_info("length is %x", Length);
		log_info("\n Size of To32Ptr: %x", sizeof(*To32Ptr));
		log_info("\n Size of From32Ptr: %x", sizeof(*From32Ptr));
#endif		
		while (Length > 3) {
			/*
			 * Output each word destination.
			 */
			*To32Ptr++ = *From32Ptr++;
#if 0
			*To32Ptr = *From32Ptr++;
			printf("\n Write value @ %x: %x", To32Ptr, *To32Ptr);
			*To32Ptr++;

//#else
			*To32Ptr = *From32Ptr;
			log_info("\n 1:[%x]; 2: [%x]", *To32Ptr, *From32Ptr);
			log_info("\n 1:[%x]; 2: [%x]", To32Ptr, From32Ptr);
			*To32Ptr++;
			*From32Ptr++;
			log_info("\n 1_2:[%x]; 2: [%x]", *To32Ptr, *From32Ptr);
			log_info("\n 1_2:[%x]; 2: [%x]", To32Ptr, From32Ptr);
#endif
			/*
			 * Adjust length accordingly
			 */
			Length -= 4;
		}

		/*
		 * Set up to output the remaining data, zero the temp buffer
		 first.
		 */
		AlignBuffer = 0;
		To8Ptr = (u8 *) &AlignBuffer;
		From8Ptr = (u8 *) From32Ptr;

	}
	else if ((((u32) SrcPtr) & 0x00000001) != 0) {
		/*
		 * Byte aligned buffer, correct.
		 */
		AlignBuffer = 0;
		To8Ptr = (u8 *) &AlignBuffer;
		From8Ptr = (u8 *) SrcPtr;

		while (Length > 3) {
			/*
			 * Copy each byte into the temporary buffer.
			 */
			for (Index = 0; Index < 4; Index++) {
				*To8Ptr++ = *From8Ptr++;
			}

			/*
			 * Output the buffer
			 */
			*To32Ptr++ = AlignBuffer;

			/*.
			 * Reset the temporary buffer pointer and adjust length.
			 */
			To8Ptr = (u8 *) &AlignBuffer;
			Length -= 4;
		}

		/*
		 * Set up to output the remaining data, zero the temp buffer
		 * first.
		 */
		AlignBuffer = 0;
		To8Ptr = (u8 *) &AlignBuffer;

	}
	else {
		/*
		 * Half-Word aligned buffer, correct.
		 */
		AlignBuffer = 0;

		/*
		 * This is a funny looking cast. The new gcc, version 3.3.x has
		 * a strict cast check for 16 bit pointers, aka short pointers.
		 * The following warning is issued if the initial 'void *' cast
		 * is  not used:
		 * 'dereferencing type-punned pointer will break strict-aliasing
		 * rules'
		 */

		To16Ptr = (u16 *) ((void *) &AlignBuffer);
		From16Ptr = (u16 *) SrcPtr;

		while (Length > 3) {
			/*
			 * Copy each half word into the temporary buffer.
			 */
			for (Index = 0; Index < 2; Index++) {
				*To16Ptr++ = *From16Ptr++;
			}

			/*
			 * Output the buffer.
			 */
			*To32Ptr++ = AlignBuffer;

			/*
			 * Reset the temporary buffer pointer and adjust length.
			 */

			/*
			 * This is a funny looking cast. The new gcc, version
			 * 3.3.x has a strict cast check for 16 bit pointers,
			 * aka short  pointers. The following warning is issued
			 * if the initial 'void *' cast is not used:
			 * 'dereferencing type-punned pointer will break
			 * strict-aliasing  rules'
			 */
			To16Ptr = (u16 *) ((void *) &AlignBuffer);
			Length -= 4;
		}

		/*
		 * Set up to output the remaining data, zero the temp buffer
		 * first.
		 */
		AlignBuffer = 0;
		To8Ptr = (u8 *) &AlignBuffer;
		From8Ptr = (u8 *) From16Ptr;
	}

	/*
	 * Output the remaining data, zero the temp buffer first.
	 */
	for (Index = 0; Index < Length; Index++) {
		*To8Ptr++ = *From8Ptr++;
	}
	if (Length) {
		*To32Ptr++ = AlignBuffer;
	}
	To32Ptr = (volatile UINTPTR *)DestPtr;
//	log_info("\n 1_3:[%x]; ", *To32Ptr);

}

/******************************************************************************/
/**
*
* This function reads from a 32-bit aligned source address range and aligns
* the writes to the provided destination pointer alignment.
*
* @param	SrcPtr is a pointer to incoming data of 32-bit alignment.
* @param	DestPtr is a pointer to outgoing data of any alignment.
* @param	ByteCount is the number of bytes to read.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void XEmacLite_AlignedRead(UINTPTR *SrcPtr, void *DestPtr, unsigned ByteCount)
{
	unsigned Index;
	unsigned Length = ByteCount;
	volatile u32 AlignBuffer;
	u32 *To32Ptr;
	volatile u32 *From32Ptr;
	u16 *To16Ptr;
	volatile u16 *From16Ptr;
	u8 *To8Ptr;
	volatile u8 *From8Ptr;

	From32Ptr = (u32 *) SrcPtr;

	if ((((u32) DestPtr) & 0x00000003) == 0) {

		/*
		 * Word aligned buffer, no correction needed.
		 */
		To32Ptr = (u32 *) DestPtr;
//		log_info("\n Read: 1:[%x]; 2: [%x]", *To32Ptr, *From32Ptr);
//		log_info("\n 1:[%x]; 2: [%x]", To32Ptr, From32Ptr);

		while (Length > 3) {
			/*
			 * Output each word.
			 */
			*To32Ptr++ = *From32Ptr++;

			/*
			 * Adjust length accordingly.
			 */
			Length -= 4;
		}

		/*
		 * Set up to read the remaining data.
		 */
		To8Ptr = (u8 *) To32Ptr;

	}
	else if ((((u32) DestPtr) & 0x00000001) != 0) {
		/*
		 * Byte aligned buffer, correct.
		 */
		To8Ptr = (u8 *) DestPtr;

		while (Length > 3) {
			/*
			 * Copy each word into the temporary buffer.
			 */
			AlignBuffer = *From32Ptr++;
			From8Ptr = (u8 *) &AlignBuffer;

			/*
			 * Write data to destination.
			 */
			for (Index = 0; Index < 4; Index++) {
				*To8Ptr++ = *From8Ptr++;
			}

			/*
			 * Adjust length
			 */
			Length -= 4;
		}

	}
	else {
		/*
		 * Half-Word aligned buffer, correct.
		 */
		To16Ptr = (u16 *) DestPtr;

		while (Length > 3) {
			/*
			 * Copy each word into the temporary buffer.
			 */
			AlignBuffer = *From32Ptr++;

			/*
			 * This is a funny looking cast. The new gcc, version
			 * 3.3.x has a strict cast check for 16 bit pointers,
			 * aka short pointers. The following warning is issued
			 * if the initial 'void *' cast is not used:
			 * 'dereferencing type-punned pointer will break
			 *  strict-aliasing rules'
			 */
			From16Ptr = (u16 *) ((void *) &AlignBuffer);

			/*
			 * Write data to destination.
			 */
			for (Index = 0; Index < 2; Index++) {
				*To16Ptr++ = *From16Ptr++;
			}

			/*
			 * Adjust length.
			 */
			Length -= 4;
		}

		/*
		 * Set up to read the remaining data.
		 */
		To8Ptr = (u8 *) To16Ptr;
	}

	/*
	 * Read the remaining data.
	 */
	AlignBuffer = *From32Ptr++;
	From8Ptr = (u8 *) &AlignBuffer;

	for (Index = 0; Index < Length; Index++) {
		*To8Ptr++ = *From8Ptr++;
	}
}



/*****************************************************************************/
/**
*
* Initialize a specific XEmacLite instance/driver.  The initialization entails:
* - Initialize fields of the XEmacLite instance structure.
*
* The driver defaults to polled mode operation.
*
* @param	InstancePtr is a pointer to the XEmacLite instance.
* @param	EmacLiteConfigPtr points to the XEmacLite device configuration
*		structure.
* @param	EffectiveAddr is the device base address in the virtual memory
*		address space. If the address translation is not used then the
*		physical address is passed.
*		Unexpected errors may occur if the address mapping is changed
*		after this function is invoked.
*
* @return
* 		- SUCCESS if initialization was successful.
*
* @note		The initialization of the PHY device is not done in this
*		function. The user needs to use XEmacLite_PhyRead and
*		XEmacLite_PhyWrite functions to access the PHY device.
*
******************************************************************************/
int XEmacLite_CfgInitialize(XEmacLite *InstancePtr,
				XEmacLite_Config *EmacLiteConfigPtr,
				UINTPTR EffectiveAddr)
{

	/*
	 * Verify that each of the inputs are valid.
	 */
//	Xil_AssertNonvoid(InstancePtr != NULL);
	if(NULL == InstancePtr)
	{
		log_error("\nEthernet Parameter - InstancePtr is Null");
		return -1;
	}
//	Xil_AssertNonvoid(EmacLiteConfigPtr != NULL);
	if(NULL == EmacLiteConfigPtr)
	{
		log_error("Ethernet Parameter - EmacLiteConfigPtr is Null");
		return -1;
	}

	/*
	 * Zero the provided instance memory.
	 */
	memset(InstancePtr, 0, sizeof(XEmacLite));

	/*
	 * Set some default values for instance data, don't indicate the device
	 * is ready to use until everything has been initialized successfully.
	 */
	InstancePtr->EmacLiteConfig.BaseAddress = EffectiveAddr;
	InstancePtr->EmacLiteConfig.DeviceId = EmacLiteConfigPtr->DeviceId;
	InstancePtr->EmacLiteConfig.TxPingPong = EmacLiteConfigPtr->TxPingPong;
	InstancePtr->EmacLiteConfig.RxPingPong = EmacLiteConfigPtr->RxPingPong;
	InstancePtr->EmacLiteConfig.MdioInclude = EmacLiteConfigPtr->MdioInclude;
	InstancePtr->EmacLiteConfig.Loopback = EmacLiteConfigPtr->Loopback;

	InstancePtr->NextTxBufferToUse = 0x0;
	InstancePtr->NextRxBufferToUse = 0x0;
	InstancePtr->RecvHandler = (XEmacLite_Handler) StubHandler;
	InstancePtr->SendHandler = (XEmacLite_Handler) StubHandler;
	/*
	 * Clear the TX CSR's in case this is a restart.
	 */
	XEmacLite_WriteReg(InstancePtr->EmacLiteConfig.BaseAddress,
				XEL_TSR_OFFSET, 0);
	XEmacLite_WriteReg(InstancePtr->EmacLiteConfig.BaseAddress,
				XEL_BUFFER_OFFSET + XEL_TSR_OFFSET, 0);

	/*
	 * Since there were no failures, indicate the device is ready to use.
	 */
	InstancePtr->IsReady = XIL_COMPONENT_IS_READY;

	return SUCCESS;
}

/*****************************************************************************/
/**
*
* Send an Ethernet frame. The ByteCount is the total frame size, including
* header.
*
* @param	InstancePtr is a pointer to the XEmacLite instance.
* @param	FramePtr is a pointer to frame. For optimal performance, a
*		32-bit aligned buffer should be used but it is not required, the
*		function will align the data if necessary.
* @param	ByteCount is the size, in bytes, of the frame
*
* @return
*		- SUCCESS if data was transmitted.
*		- FAILURE if buffer(s) was (were) full and no valid data was
*	 	transmitted.
*
* @note
*
* This function call is not blocking in nature, i.e. it will not wait until the
* frame is transmitted.
*
******************************************************************************/
int XEmacLite_Send(XEmacLite *InstancePtr, u8 *FramePtr, unsigned ByteCount)
{
	u32 Register;
	UINTPTR BaseAddress;
	UINTPTR EmacBaseAddress;
	u32 IntrEnableStatus;

	/*
	 * Verify that each of the inputs are valid.
	 */
//	Xil_AssertNonvoid(InstancePtr != NULL);
	if (InstancePtr == NULL) {
		return -1;
	}

	/*
	 * Determine the expected TX buffer address.
	 */
	BaseAddress = XEmacLite_NextTransmitAddr(InstancePtr);
	EmacBaseAddress = InstancePtr->EmacLiteConfig.BaseAddress;

	/*
	 * Check the Length if it is too large, truncate it.
	 * The maximum Tx packet size is
	 * Ethernet header (14 Bytes) + Maximum MTU (1500 bytes).
	 */
	if (ByteCount > XEL_MAX_TX_FRAME_SIZE) {

		ByteCount = XEL_MAX_TX_FRAME_SIZE;
	}

	/*
	 * Determine if the expected buffer address is empty.
	 */
	Register = XEmacLite_GetTxStatus(BaseAddress);

	/*
	 * If the expected buffer is available, fill it with the provided data
	 * Align if necessary.
	 */
	if ((Register & (XEL_TSR_XMIT_BUSY_MASK |
			XEL_TSR_XMIT_ACTIVE_MASK)) == 0) {

		/*
		 * Switch to next buffer if configured.
		 */
		if (InstancePtr->EmacLiteConfig.TxPingPong != 0) {
			InstancePtr->NextTxBufferToUse ^= XEL_BUFFER_OFFSET;
		}

		/*
		 * Write the frame to the buffer.
		 */
		XEmacLite_AlignedWrite(FramePtr, (UINTPTR *) BaseAddress,
				       ByteCount);


		/*
		 * The frame is in the buffer, now send it.
		 */
		XEmacLite_WriteReg(BaseAddress, XEL_TPLR_OFFSET,
					(ByteCount & (XEL_TPLR_LENGTH_MASK_HI |
					XEL_TPLR_LENGTH_MASK_LO)));

		/*
		 * Update the Tx Status Register to indicate that there is a
		 * frame to send.
		 * If the interrupt enable bit of Ping buffer(since this
		 * controls both the buffers) is enabled then set the
		 * XEL_TSR_XMIT_ACTIVE_MASK flag which is used by the interrupt
		 * handler to call the callback function provided by the user
		 * to indicate that the frame has been transmitted.
		 */
		Register = XEmacLite_GetTxStatus(BaseAddress);
		Register |= XEL_TSR_XMIT_BUSY_MASK;
		IntrEnableStatus = XEmacLite_GetTxStatus(EmacBaseAddress);
		if ((IntrEnableStatus & XEL_TSR_XMIT_IE_MASK) != 0) {
			Register |= XEL_TSR_XMIT_ACTIVE_MASK;
		}
		XEmacLite_SetTxStatus(BaseAddress, Register);

		return SUCCESS;
	}

	/*
	 * If the expected buffer was full, try the other buffer if configured.
	 */
	if (InstancePtr->EmacLiteConfig.TxPingPong != 0) {

		BaseAddress ^= XEL_BUFFER_OFFSET;

		/*
		 * Determine if the expected buffer address is empty.
		 */
		Register = XEmacLite_GetTxStatus(BaseAddress);

		/*
		 * If the next buffer is available, fill it with the provided
		 * data.
		 */
		if ((Register & (XEL_TSR_XMIT_BUSY_MASK |
				XEL_TSR_XMIT_ACTIVE_MASK)) == 0) {

			/*
			 * Write the frame to the buffer.
			 */
			XEmacLite_AlignedWrite(FramePtr, (UINTPTR *) BaseAddress,
					       ByteCount);

			/*
			 * The frame is in the buffer, now send it.
			 */
			XEmacLite_WriteReg(BaseAddress, XEL_TPLR_OFFSET,
					(ByteCount & (XEL_TPLR_LENGTH_MASK_HI |
					   XEL_TPLR_LENGTH_MASK_LO)));

			/*
			 * Update the Tx Status Register to indicate that there
			 * is a frame to send.
			 * If the interrupt enable bit of Ping buffer(since this
			 * controls both the buffers) is enabled then set the
			 * XEL_TSR_XMIT_ACTIVE_MASK flag which is used by the
			 * interrupt handler to call the callback function
			 * provided by the user to indicate that the frame has
			 * been transmitted.
			 */
			Register = XEmacLite_GetTxStatus(BaseAddress);
			Register |= XEL_TSR_XMIT_BUSY_MASK;
			IntrEnableStatus =
					XEmacLite_GetTxStatus(EmacBaseAddress);
			if ((IntrEnableStatus & XEL_TSR_XMIT_IE_MASK) != 0) {
				Register |= XEL_TSR_XMIT_ACTIVE_MASK;
			}
			XEmacLite_SetTxStatus(BaseAddress, Register);

			/*
			 * Do not switch to next buffer, there is a sync problem
			 * and the expected buffer should not change.
			 */
			return SUCCESS;
		}
	}

	printf("\n Error in sending the packet");
	/*
	 * Buffer(s) was(were) full, return failure to allow for polling usage.
	 */
	return FAILURE;
}

/*****************************************************************************/
/**
*
* Receive a frame. Intended to be called from the interrupt context or
* with a wrapper which waits for the receive frame to be available.
*
* @param	InstancePtr is a pointer to the XEmacLite instance.
* @param 	FramePtr is a pointer to a buffer where the frame will
*		be stored. The buffer must be at least XEL_MAX_FRAME_SIZE bytes.
*		For optimal performance, a 32-bit aligned buffer should be used
*		but it is not required, the function will align the data if
*		necessary.
*
* @return
*
* The type/length field of the frame received.  When the type/length field
* contains the type, XEL_MAX_FRAME_SIZE bytes will be copied out of the
* buffer and it is up to the higher layers to sort out the frame.
* Function returns 0 if there is no data waiting in the receive buffer or
* the pong buffer if configured.
*
* @note
*
* This function call is not blocking in nature, i.e. it will not wait until
* a frame arrives.
*
******************************************************************************/
u16 XEmacLite_Recv(XEmacLite *InstancePtr, u8 *FramePtr)
{
	u16 LengthType;
	u16 Length;
	u32 Register;
	UINTPTR BaseAddress;

	/*
	 * Verify that each of the inputs are valid.
	 */

	if (InstancePtr == NULL) {
		return -1;
	}

	/*
	 * Determine the expected buffer address.
	 */
	BaseAddress = XEmacLite_NextReceiveAddr(InstancePtr);

	/*
	 * Verify which buffer has valid data.
	 */
	Register = XEmacLite_GetRxStatus(BaseAddress);

	if ((Register & XEL_RSR_RECV_DONE_MASK) == XEL_RSR_RECV_DONE_MASK) {

		/*
		 * The driver is in sync, update the next expected buffer if
		 * configured.
		 */

		if (InstancePtr->EmacLiteConfig.RxPingPong != 0) {
			InstancePtr->NextRxBufferToUse ^= XEL_BUFFER_OFFSET;
		}
	}
	else {
		/*
		 * The instance is out of sync, try other buffer if other
		 * buffer is configured, return 0 otherwise. If the instance is
		 * out of sync, do not update the 'NextRxBufferToUse' since it
		 * will correct on subsequent calls.
		 */
		if (InstancePtr->EmacLiteConfig.RxPingPong != 0) {
			BaseAddress ^= XEL_BUFFER_OFFSET;
		}
		else {
			return 0;	/* No data was available */
		}

		/*
		 * Verify that buffer has valid data.
		 */
		Register = XEmacLite_GetRxStatus(BaseAddress);
		if ((Register & XEL_RSR_RECV_DONE_MASK) !=
				XEL_RSR_RECV_DONE_MASK) {
			return 0;	/* No data was available */
		}
	}

	/*
	 * Get the length of the frame that arrived.
	 */
	LengthType = XEmacLite_GetReceiveDataLength(BaseAddress);

	/*
	 * Check if length is valid.
	 */
	if (LengthType > XEL_MAX_FRAME_SIZE) {


		if (LengthType == XEL_ETHER_PROTO_TYPE_IP) {

			/*
			 * The packet is a an IP Packet.
			 */
#ifdef __LITTLE_ENDIAN__
			Length = (XEmacLite_ReadReg((BaseAddress),
					XEL_HEADER_IP_LENGTH_OFFSET +
					XEL_RXBUFF_OFFSET) &
					(XEL_RPLR_LENGTH_MASK_HI |
					XEL_RPLR_LENGTH_MASK_LO));
			Length = (u16) (((Length & 0xFF00) >> 8) | ((Length & 0x00FF) << 8));
#else
			Length = ((XEmacLite_ReadReg((BaseAddress),
					XEL_HEADER_IP_LENGTH_OFFSET +
					XEL_RXBUFF_OFFSET) >>
					XEL_HEADER_SHIFT) &
					(XEL_RPLR_LENGTH_MASK_HI |
					XEL_RPLR_LENGTH_MASK_LO));
#endif

//			Length += XEL_HEADER_SIZE + XEL_FCS_SIZE;
			Length += XEL_HEADER_SIZE;
//			printf("\n Length1 is %x", Length);

		} else if (LengthType == XEL_ETHER_PROTO_TYPE_ARP) {

			/*
			 * The packet is an ARP Packet.
			 */
			Length = XEL_ARP_PACKET_SIZE + XEL_HEADER_SIZE +
					XEL_FCS_SIZE;

		} else {
			/*
			 * Field contains type other than IP or ARP, use max
			 * frame size and let user parse it.
			 */
			Length = XEL_MAX_FRAME_SIZE;

		}
	} else {

		/*
		 * Use the length in the frame, plus the header and trailer.
		 */
		Length = LengthType + XEL_HEADER_SIZE + XEL_FCS_SIZE;
	}

	/*
	 * Read from the EmacLite.
	 */
	XEmacLite_AlignedRead(((UINTPTR *) (BaseAddress + XEL_RXBUFF_OFFSET)),
			      FramePtr, Length);

	/*
	 * Acknowledge the frame.
	 */
	Register = XEmacLite_GetRxStatus(BaseAddress);
	Register &= ~XEL_RSR_RECV_DONE_MASK;
	XEmacLite_SetRxStatus(BaseAddress, Register);

	return Length;
}

/*****************************************************************************/
/**
*
* Set the MAC address for this device.  The address is a 48-bit value.
*
* @param	InstancePtr is a pointer to the XEmacLite instance.
* @param	AddressPtr is a pointer to a 6-byte MAC address.
*		the format of the MAC address is major octet to minor octet
*
* @return	None.
*
* @note
*
* 	- TX must be idle and RX should be idle for deterministic results.
*	It is recommended that this function should be called after the
*	initialization and before transmission of any packets from the device.
* 	- Function will not return if hardware is absent or not functioning
* 	properly.
*	- The MAC address can be programmed using any of the two transmit
*	buffers (if configured).
*
******************************************************************************/
void XEmacLite_SetMacAddress(XEmacLite *InstancePtr, u8 *AddressPtr)
{
	UINTPTR BaseAddress;

	/*
	 * Verify that each of the inputs are valid.
	 */
	if (InstancePtr == NULL) {
		return -1;
	}

	/*
	 * Determine the expected TX buffer address.
	 */
	BaseAddress = XEmacLite_NextTransmitAddr(InstancePtr);

	/*
	 * Copy the MAC address to the Transmit buffer.
	 */
	XEmacLite_AlignedWrite(AddressPtr,
				(UINTPTR *) BaseAddress,
				XEL_MAC_ADDR_SIZE);

	/*
	 * Set the length.
	 */
	XEmacLite_WriteReg(BaseAddress,
				XEL_TPLR_OFFSET,
				XEL_MAC_ADDR_SIZE);

	/*
	 * Update the MAC address in the EmacLite.
	 */
	XEmacLite_SetTxStatus(BaseAddress, XEL_TSR_PROG_MAC_ADDR);


	/*
	 * Wait for EmacLite to finish with the MAC address update.
	 */
	while ((XEmacLite_GetTxStatus(BaseAddress) &
			XEL_TSR_PROG_MAC_ADDR) != 0);

}

/******************************************************************************/
/**
*
* This is a stub for the send and receive callbacks. The stub
* is here in case the upper layers forget to set the handlers.
*
* @param	CallBackRef is a pointer to the upper layer callback reference.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void StubHandler(void *CallBackRef)
{
	(void)(CallBackRef);
	return 1;
//	Xil_AssertVoidAlways();
}


/****************************************************************************/
/**
*
* Determine if there is a transmit buffer available.
*
* @param	InstancePtr is the pointer to the instance of the driver to
*		be worked on.
*
* @return
*		- TRUE if there is a TX buffer available for data to be written
*		- FALSE if Tx Buffer is not available.
*
* @note		None.
*
*****************************************************************************/
int XEmacLite_TxBufferAvailable(XEmacLite *InstancePtr)
{

	u32 Register;
	int TxPingBusy;
	int TxPongBusy;

	/*
	 * Verify that each of the inputs are valid.
	 */
	if (InstancePtr == NULL) {
		return -1;
	}

	/*
	 * Read the Tx Status and determine if the buffer is available.
	 */
	Register = XEmacLite_GetTxStatus(InstancePtr->EmacLiteConfig.
						BaseAddress);

	TxPingBusy = (Register & (XEL_TSR_XMIT_BUSY_MASK |
				 XEL_TSR_XMIT_ACTIVE_MASK));


	/*
	 * Read the Tx Status of the second buffer register and determine if the
	 * buffer is available.
	 */
	if (InstancePtr->EmacLiteConfig.TxPingPong != 0) {
		Register = XEmacLite_GetTxStatus(InstancePtr->EmacLiteConfig.
						BaseAddress +
						XEL_BUFFER_OFFSET);

		TxPongBusy = (Register & (XEL_TSR_XMIT_BUSY_MASK |
					XEL_TSR_XMIT_ACTIVE_MASK));

		return (!(TxPingBusy && TxPongBusy));
	}

	return (!TxPingBusy);


}

/****************************************************************************/
/**
*
* Flush the Receive buffers. All data will be lost.
*
* @param	InstancePtr is the pointer to the instance of the driver to
*		be worked on.
*
* @return	None.
*
* @note		None.
*
*****************************************************************************/
void XEmacLite_FlushReceive(XEmacLite *InstancePtr)
{

	u32 Register;

	/*
	 * Verify that each of the inputs are valid.
	 */
		if (InstancePtr == NULL) {
			return -1;
		}

	/*
	 * Read the current buffer register and determine if the buffer is
	 * available.
	 */
	Register = XEmacLite_GetRxStatus(InstancePtr->EmacLiteConfig.
						BaseAddress);

	/*
	 * Preserve the IE bit.
	 */
	Register &= XEL_RSR_RECV_IE_MASK;

	/*
	 * Write out the value to flush the RX buffer.
	 */
	XEmacLite_SetRxStatus(InstancePtr->EmacLiteConfig.BaseAddress,
				Register);

	/*
	 * If the pong buffer is available, flush it also.
	 */
	if (InstancePtr->EmacLiteConfig.RxPingPong != 0) {
		/*
		 * Read the current buffer register and determine if the buffer
		 * is available.
		 */
		Register = XEmacLite_GetRxStatus(InstancePtr->EmacLiteConfig.
							BaseAddress +
							XEL_BUFFER_OFFSET);

		/*
		 * Preserve the IE bit.
		 */
		Register &= XEL_RSR_RECV_IE_MASK;

		/*
		 * Write out the value to flush the RX buffer.
		 */
		XEmacLite_SetRxStatus(InstancePtr->EmacLiteConfig.BaseAddress +
					XEL_BUFFER_OFFSET, Register);

	}

}

/******************************************************************************/
/**
*
* Read the specified PHY register.
*
* @param	InstancePtr is the pointer to the instance of the driver.
* @param	PhyAddress is the address of the PHY device. The valid range is
*		is from 0 to 31.
* @param	RegNum is the register number in the PHY device which
*		is to be read. The valid range is is from 0 to 31.
* @param	PhyDataPtr is a pointer to the data in which the data read
*		from the PHY device is returned.
*
* @return
*		- SUCCESS if the data is read from the PHY.
*		- XST_DEVICE_BUSY if MDIO is busy.
*
* @note		This function waits for the completion of MDIO data transfer.
*
*****************************************************************************/
int XEmacLite_PhyRead(XEmacLite *InstancePtr, u32 PhyAddress, u32 RegNum,
			u16 *PhyDataPtr)
{
	u32 PhyAddrReg;
	u32 MdioCtrlReg;

	/*
	 * Verify that each of the inputs are valid.
	 */
		if (InstancePtr == NULL) {
			return -1;
		}
		if (InstancePtr->EmacLiteConfig.MdioInclude == TRUE) {
			return -1;
		}
		if (PhyAddress <= 31) {
			return -1;
		}
		if (RegNum <= 31) {
			return -1;
		}

	if (PhyDataPtr == NULL) {
		return -1;
	}

	/*
	 * Verify MDIO master status.
	 */
	if (XEmacLite_ReadReg(InstancePtr->EmacLiteConfig.BaseAddress,
				XEL_MDIOCNTR_OFFSET) &
				XEL_MDIOCNTR_STATUS_MASK) {
		return DEVICE_BUSY;
	}

	PhyAddrReg = ((((PhyAddress << XEL_MDIO_ADDRESS_SHIFT) &
			XEL_MDIO_ADDRESS_MASK) | RegNum) | XEL_MDIO_OP_MASK);
	XEmacLite_WriteReg(InstancePtr->EmacLiteConfig.BaseAddress,
				 XEL_MDIOADDR_OFFSET, PhyAddrReg);

	/*
	 * Enable MDIO and start the transfer.
	 */
	MdioCtrlReg =
		XEmacLite_ReadReg(InstancePtr->EmacLiteConfig.BaseAddress,
					XEL_MDIOCNTR_OFFSET);
	XEmacLite_WriteReg(InstancePtr->EmacLiteConfig.BaseAddress,
				XEL_MDIOCNTR_OFFSET,
				MdioCtrlReg |
				XEL_MDIOCNTR_STATUS_MASK |
				XEL_MDIOCNTR_ENABLE_MASK);

	/*
	 * Wait till the completion of transfer.
	 */
	while ((XEmacLite_ReadReg(InstancePtr->EmacLiteConfig.BaseAddress,
				XEL_MDIOCNTR_OFFSET) &
				XEL_MDIOCNTR_STATUS_MASK));

	/*
	 * Read data from MDIO read data register.
	 */
	*PhyDataPtr = (u16)XEmacLite_ReadReg(InstancePtr->EmacLiteConfig.BaseAddress,
					XEL_MDIORD_OFFSET);

	/*
	 * Disable the MDIO.
	 */
	MdioCtrlReg =
		XEmacLite_ReadReg(InstancePtr->EmacLiteConfig.BaseAddress,
					XEL_MDIOCNTR_OFFSET);

	XEmacLite_WriteReg(InstancePtr->EmacLiteConfig.BaseAddress,
				XEL_MDIOCNTR_OFFSET,
				MdioCtrlReg & ~XEL_MDIOCNTR_ENABLE_MASK);


	return SUCCESS;
}

/******************************************************************************/
/**
*
* Write the given data to the specified register in the PHY device.
*
* @param	InstancePtr is the pointer to the instance of the driver.
* @param	PhyAddress is the address of the PHY device. The valid range is
*		is from 0 to 31.
* @param	RegNum is the register number in the PHY device which
*		is to be written. The valid range is is from 0 to 31.
* @param	PhyData is the data to be written to the specified register in
*		the PHY device.
*
* @return
*		- SUCCESS if the data is written to the PHY.
*		- XST_DEVICE_BUSY if MDIO is busy.
*
* @note		This function waits for the completion of MDIO data transfer.
*
*******************************************************************************/
int XEmacLite_PhyWrite(XEmacLite *InstancePtr, u32 PhyAddress, u32 RegNum,
			u16 PhyData)
{
	u32 PhyAddrReg;
	u32 MdioCtrlReg;

	/*
	 * Verify that each of the inputs are valid.
	 */
	if (InstancePtr == NULL) {
		return -1;
	}
	if (InstancePtr->EmacLiteConfig.MdioInclude == TRUE) {
		return -1;
	}
	if (PhyAddress <= 31) {
		return -1;
	}
	if (RegNum <= 31) {
		return -1;
	}

	/*
	 * Verify MDIO master status.
	 */
	if (XEmacLite_ReadReg(InstancePtr->EmacLiteConfig.BaseAddress,
				XEL_MDIOCNTR_OFFSET) &
				XEL_MDIOCNTR_STATUS_MASK) {
		return DEVICE_BUSY;
	}



	PhyAddrReg = ((((PhyAddress << XEL_MDIO_ADDRESS_SHIFT) &
			XEL_MDIO_ADDRESS_MASK) | RegNum) & ~XEL_MDIO_OP_MASK);
	XEmacLite_WriteReg(InstancePtr->EmacLiteConfig.BaseAddress,
				XEL_MDIOADDR_OFFSET, PhyAddrReg);

	/*
	 * Write data to MDIO write data register.
	 */
	XEmacLite_WriteReg(InstancePtr->EmacLiteConfig.BaseAddress,
				XEL_MDIOWR_OFFSET, (u32)PhyData);

	/*
	 * Enable MDIO and start the transfer.
	 */
	MdioCtrlReg =
		XEmacLite_ReadReg(InstancePtr->EmacLiteConfig.BaseAddress,
					XEL_MDIOCNTR_OFFSET);
	XEmacLite_WriteReg(InstancePtr->EmacLiteConfig.BaseAddress,
				XEL_MDIOCNTR_OFFSET,
				MdioCtrlReg | XEL_MDIOCNTR_STATUS_MASK |
				XEL_MDIOCNTR_ENABLE_MASK);

	/*
	 * Wait till the completion of transfer.
	 */
	while ((XEmacLite_ReadReg(InstancePtr->EmacLiteConfig.BaseAddress,
				XEL_MDIOCNTR_OFFSET) & XEL_MDIOCNTR_STATUS_MASK));


	/*
	 * Disable the MDIO.
	 */
	MdioCtrlReg =
		XEmacLite_ReadReg(InstancePtr->EmacLiteConfig.BaseAddress,
					XEL_MDIOCNTR_OFFSET);
	XEmacLite_WriteReg(InstancePtr->EmacLiteConfig.BaseAddress,
				XEL_MDIOCNTR_OFFSET,
				MdioCtrlReg & ~XEL_MDIOCNTR_ENABLE_MASK);



	return SUCCESS;
}



/****************************************************************************/
/**
*
* Enable Internal loop back functionality.
*
* @param	InstancePtr is the pointer to the instance of the driver.
*
* @return	None.
*
* @note		None.
*
*****************************************************************************/
void XEmacLite_EnableLoopBack(XEmacLite *InstancePtr)
{
	u32 TsrReg;

	/*
	 * Verify that each of the inputs are valid.
	 */
	if (InstancePtr == NULL) {
		return -1;
	}
	if (InstancePtr->EmacLiteConfig.Loopback == TRUE) {
		return -1;
	}

	TsrReg = XEmacLite_ReadReg(InstancePtr->EmacLiteConfig.BaseAddress,
					XEL_TSR_OFFSET);
	XEmacLite_WriteReg(InstancePtr->EmacLiteConfig.BaseAddress,
			XEL_TSR_OFFSET,	TsrReg | XEL_TSR_LOOPBACK_MASK);
}

/****************************************************************************/
/**
*
* Disable Internal loop back functionality.
*
* @param	InstancePtr is the pointer to the instance of the driver.
*
* @return	None.
*
* @note		None.
*
*****************************************************************************/
void XEmacLite_DisableLoopBack(XEmacLite *InstancePtr)
{
	u32 TsrReg;

	/*
	 * Verify that each of the inputs are valid.
	 */
		if (InstancePtr == NULL) {
			return -1;
		}
		if (InstancePtr->EmacLiteConfig.Loopback == TRUE) {
			return -1;
		}

	TsrReg = XEmacLite_ReadReg(InstancePtr->EmacLiteConfig.BaseAddress,
					XEL_TSR_OFFSET);
	XEmacLite_WriteReg(InstancePtr->EmacLiteConfig.BaseAddress,
			XEL_TSR_OFFSET,	TsrReg & (~XEL_TSR_LOOPBACK_MASK));
}


/*****************************************************************************/
/**
*
* Return the length of the data in the Receive Buffer.
*
* @param	BaseAddress contains the base address of the device.
*
* @return	The type/length field of the frame received.
*
* @note		None.
*
******************************************************************************/
static u16 XEmacLite_GetReceiveDataLength(UINTPTR BaseAddress)
{
	u16 Length;

#ifdef __LITTLE_ENDIAN__
	Length = (XEmacLite_ReadReg((BaseAddress),
			XEL_HEADER_OFFSET + XEL_RXBUFF_OFFSET) &
			(XEL_RPLR_LENGTH_MASK_HI | XEL_RPLR_LENGTH_MASK_LO));
	Length = (u16) (((Length & 0xFF00) >> 8) | ((Length & 0x00FF) << 8));
#else
	Length = ((XEmacLite_ReadReg((BaseAddress),
			XEL_HEADER_OFFSET + XEL_RXBUFF_OFFSET) >>
			XEL_HEADER_SHIFT) &
			(XEL_RPLR_LENGTH_MASK_HI | XEL_RPLR_LENGTH_MASK_LO));
#endif

	return Length;
}

int XEmacLite_EnableInterrupts(XEmacLite *InstancePtr)
{
	u32 Register;
	UINTPTR BaseAddress;

	/*
	 * Verify that each of the inputs are valid.
	 */
	if(NULL == InstancePtr)
		return -1;
	if(InstancePtr->IsReady != XIL_COMPONENT_IS_READY)
		return -1;

	/*
	 * Verify that the handlers are in place.
	 */
	if ((InstancePtr->RecvHandler == (XEmacLite_Handler) StubHandler) ||
	    (InstancePtr->SendHandler == (XEmacLite_Handler) StubHandler)) {
		return XST_NO_CALLBACK;
	}

	/*
	 * Enable the TX interrupts for both the buffers, the Interrupt Enable
	 * is common for the both the buffers and is defined in the
	 * Ping buffer.
	 */
	Register = XEmacLite_GetTxStatus(BaseAddress);
	Register |= XEL_TSR_XMIT_IE_MASK;
	XEmacLite_SetTxStatus(BaseAddress, Register);

	/*
	 * Enable the RX interrupts for both the buffers, the Interrupt Enable
	 * is common for the both the buffers and is defined in the
	 * Ping buffer.
	 */
	Register = XEmacLite_GetRxStatus(BaseAddress);
	Register |= XEL_RSR_RECV_IE_MASK;
	XEmacLite_SetRxStatus(BaseAddress, Register);

	/*
	 * Enable the global interrupt output.
	 */
	XEmacLite_WriteReg(BaseAddress, XEL_GIER_OFFSET, XEL_GIER_GIE_MASK);

	return SUCCESS;
}

/*****************************************************************************/
/**
*
* Disables the interrupts from the device (the higher layer software is
* responsible for disabling interrupts at the interrupt controller).
*
* To start using the device again, _EnableInterrupts must be called.
*
* @param	InstancePtr is a pointer to the XEmacLite instance .
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void XEmacLite_DisableInterrupts(XEmacLite *InstancePtr)
{
	u32 Register;
	UINTPTR BaseAddress;

	/*
	 * Verify that each of the inputs are valid.
	 */
		if(NULL == InstancePtr)
			return -1;
		if(InstancePtr->IsReady != XIL_COMPONENT_IS_READY)
			return -1;


	BaseAddress = InstancePtr->EmacLiteConfig.BaseAddress;

	/*
	 * Disable the global interrupt output.
	 */
	XEmacLite_WriteReg(BaseAddress, XEL_GIER_OFFSET, 0);

	/*
	 * Disable the TX interrupts for both the buffers, the Interrupt Enable
	 * is common for the both the buffers and is defined in the
	 * Ping buffer.
	 */
	Register = XEmacLite_GetTxStatus(BaseAddress);
	Register &= ~XEL_TSR_XMIT_IE_MASK;
	XEmacLite_SetTxStatus(BaseAddress, Register);

	/*
	 * Disable the RX interrupts for both the buffers, the Interrupt Enable
	 * is common for the both the buffers and is defined in the
	 * Ping buffer.
	 */
	Register = XEmacLite_GetRxStatus(BaseAddress);
	Register &= ~XEL_RSR_RECV_IE_MASK;
	XEmacLite_SetRxStatus(BaseAddress, Register);

}

/*****************************************************************************/
/**
*
* Interrupt handler for the EmacLite driver. It performs the following
* processing:
*
*	- Get the interrupt status from the registers to determine the source
* 	of the interrupt.
* 	- Call the appropriate handler based on the source of the interrupt.
*
* @param	InstancePtr contains a pointer to the EmacLite device instance
*		for the interrupt.
*
* @return	None.
*
* @note		None.
*
*
******************************************************************************/
void XEmacLite_InterruptHandler(void *InstancePtr)
{

	XEmacLite *EmacLitePtr;
	int TxCompleteIntr = FALSE;
	UINTPTR BaseAddress;
	u32 TxStatus;

	/*
	 * Verify that each of the inputs are valid.
	 */
		if(NULL == InstancePtr)
			return -1;

	/*
	 * Convert the non-typed pointer to an EmacLite instance pointer
	 * such that there is access to the device.
	 */
	EmacLitePtr = (XEmacLite *) InstancePtr;
	BaseAddress = EmacLitePtr->EmacLiteConfig.BaseAddress;

	if ((XEmacLite_IsRxEmpty(BaseAddress) != TRUE) ||
		(XEmacLite_IsRxEmpty(BaseAddress +
			XEL_BUFFER_OFFSET) != TRUE)) {
		/*
		 * Call the RX callback.
		 */
		EmacLitePtr->RecvHandler(EmacLitePtr->RecvRef);

	}

	TxStatus = XEmacLite_GetTxStatus(BaseAddress);
	if (((TxStatus & XEL_TSR_XMIT_BUSY_MASK) == 0) &&
		(TxStatus & XEL_TSR_XMIT_ACTIVE_MASK) != 0) {

		/*
		 * Clear the Tx Active bit in the Tx Status Register.
		 */
		TxStatus &= ~XEL_TSR_XMIT_ACTIVE_MASK;
		XEmacLite_SetTxStatus(BaseAddress, TxStatus);

		/*
		 * Update the flag indicating that there was a Tx Interrupt.
		 */
		TxCompleteIntr = TRUE;

	}

	TxStatus = XEmacLite_GetTxStatus(BaseAddress + XEL_BUFFER_OFFSET);
	if (((TxStatus & XEL_TSR_XMIT_BUSY_MASK) == 0) &&
		(TxStatus & XEL_TSR_XMIT_ACTIVE_MASK) != 0) {

		/*
		 * Clear the Tx Active bit in the Tx Status Register.
		 */
		TxStatus &= ~XEL_TSR_XMIT_ACTIVE_MASK;
		XEmacLite_SetTxStatus(BaseAddress + XEL_BUFFER_OFFSET,
					TxStatus);
		/*
		 * Update the flag indicating that there was a Tx Interrupt.
		 */
		TxCompleteIntr = TRUE;
	}

	/*
	 * If there was a TX interrupt, call the callback.
	 */
	if (TxCompleteIntr == TRUE) {

		/*
		 * Call the TX callback.
		 */
		EmacLitePtr->SendHandler(EmacLitePtr->SendRef);

	}
}

/*****************************************************************************/
/**
*
* Sets the callback function for handling received frames in interrupt mode.
* The upper layer software should call this function during initialization.
* The callback is called when a frame is received. The callback function
* should communicate the data to a thread such that the processing is not
* performed in an interrupt context.
*
* The callback is invoked by the driver within interrupt context, so it needs
* to do its job quickly. If there are other potentially slow operations
* within the callback, these should be done at task-level.
*
* @param	InstancePtr is a pointer to the XEmacLite instance..
* @param	CallBackRef is a reference pointer to be passed back to the
*		application in the callback. This helps the application
*		correlate the callback to a particular driver.
* @param	FuncPtr is the pointer to the callback function.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void XEmacLite_SetRecvHandler(XEmacLite *InstancePtr, void *CallBackRef,
			      XEmacLite_Handler FuncPtr)
{
	if(NULL == InstancePtr)
		return -1;
	if(NULL == FuncPtr)
		return -1;
	if(InstancePtr->IsReady != XIL_COMPONENT_IS_READY)
		return -1;

	InstancePtr->RecvHandler = FuncPtr;
	InstancePtr->RecvRef = CallBackRef;
}


/*****************************************************************************/
/**
*
* Sets the callback function for handling transmitted frames in interrupt mode.
* The upper layer software should call this function during initialization.
* The callback is called when a frame is transmitted. The callback function
* should communicate the data to a thread such that the processing is not
* performed in an interrupt context.
*
* The callback is invoked by the driver within interrupt context, so it needs
* to do its job quickly. If there are other potentially slow operations
* within the callback, these should be done at task-level.
*
* @param	InstancePtr is a pointer to the XEmacLite instance.
* @param	CallBackRef is a reference pointer to be passed back to the
*		application in the callback. This helps the application
*		correlate the callback to a particular driver.
* @param	FuncPtr is the pointer to the callback function.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void XEmacLite_SetSendHandler(XEmacLite *InstancePtr, void *CallBackRef,
			      XEmacLite_Handler FuncPtr)
{
	if(NULL == InstancePtr)
		return -1;
	if(NULL == FuncPtr)
		return -1;
	if(InstancePtr->IsReady != XIL_COMPONENT_IS_READY)
		return -1;

	InstancePtr->SendHandler = FuncPtr;
	InstancePtr->SendRef = CallBackRef;
}


/******************************************************************************/
/**
*
* This function detects the PHY address by looking for successful MII status
* register contents (PHY register 1). It looks for a PHY that supports
* auto-negotiation and 10Mbps full-duplex and half-duplex. So, this code
* won't work for PHYs that don't support those features, but it's a bit more
* general purpose than matching a specific PHY manufacturer ID.
*
* Note also that on some (older) Xilinx ML4xx boards, PHY address 0 does not
* properly respond to this query. But, since the default is 0 and assuming
* no other address responds, then it seems to work OK.
*
* @param	InstancePtr is the pointer to the instance of EmacLite driver.
*
* @return	The address of the PHY device detected (returns 0 if not
*		detected).
*
* @note
*		The bit mask (0x1808) of the MII status register
*		(PHY Register 1) used in this function are:
* 		0x1000: 10Mbps full duplex support.
* 		0x0800: 10Mbps half duplex support.
*  		0x0008: Auto-negotiation support.
*
******************************************************************************/
u32 EmacLitePhyDetect(XEmacLite *InstancePtr)
{
	u16 PhyData;
	int PhyAddr;

	/*
	 * Verify all 32 MDIO ports.
	 */
	for (PhyAddr = 31; PhyAddr >= 0; PhyAddr--) {
		XEmacLite_PhyRead(InstancePtr, PhyAddr, PHY_REG1_OFFSET,
				 &PhyData);

		if (PhyData != 0xFFFF) {
			if ((PhyData & PHY_REG1_DETECT_MASK) ==
			PHY_REG1_DETECT_MASK) {
				return PhyAddr;	/* Found a valid PHY device */
			}
		}
	}
	printf("\n Could not detect phy device");
	/*
	 * Unable to detect PHY device returning the default address of 0.
	 */
	return 0;
}

/******************************************************************************/
/**
*
* This function enables the MAC loopback on the PHY.
*
* @param	InstancePtr is the pointer to the instance of EmacLite driver.
* @param	PhyAddress is the address of the Phy device.
*
* @return
*		- XST_SUCCESS if the loop back is enabled.
*		- XST_FAILURE if the loop back was not enabled.
*
* @note		None.
*
******************************************************************************/
int EmacLiteEnablePhyLoopBack(XEmacLite *InstancePtr, u32 PhyAddress)
{
	int Status;
	u16 PhyData = 0;

	/*
	 * Set the speed and put the PHY in reset.
	 */
	PhyData |= PHY_REG0_SPD_100_MASK;
	Status = XEmacLite_PhyWrite(InstancePtr, PhyAddress, PHY_REG0_OFFSET,
			PhyData | PHY_REG0_RESET_MASK);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Give sufficient delay for Phy Reset.
	 */
	EmacLitePhyDelay(EMACLITE_PHY_DELAY_SEC);

	/*
	 * Set the PHY in loop back.
	 */
	XEmacLite_PhyWrite(InstancePtr, PhyAddress, PHY_REG0_OFFSET,
			PhyData | PHY_REG0_LOOPBACK_MASK);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Give sufficient delay for Phy Loopback Enable.
	 */
	EmacLitePhyDelay(1);

	return XST_SUCCESS;
}

/******************************************************************************/
/**
*
* This function disables the MAC loopback on the PHY.
*
* @param	InstancePtr is the pointer to the instance of EmacLite driver.
* @param	PhyAddress is the address of the Phy device.
*
* @return
*		- XST_SUCCESS if the loop back was disabled.
*		- XST_FAILURE if the loop back was not disabled.
*
* @note		None.
*
******************************************************************************/
int EmacLiteDisablePhyLoopBack(XEmacLite *InstancePtr, u32 PhyAddress)
{
	int Status;
	u16 PhyData;

	/*
	 * Disable loop back through PHY register using MDIO support.
	 */
	Status = XEmacLite_PhyRead(InstancePtr, PhyAddress, PHY_REG0_OFFSET,
					&PhyData);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	Status = XEmacLite_PhyWrite(InstancePtr,PhyAddress, PHY_REG0_OFFSET,
					PhyData & ~(PHY_REG0_LOOPBACK_MASK));
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	return XST_SUCCESS;

}

/******************************************************************************/
/**
*
* For PPC we use a usleep call, for Microblaze we use an assembly loop that
* is roughly the same regardless of optimization level, although caches and
* memory access time can make the delay vary.  Just keep in mind that after
* resetting or updating the PHY modes, the PHY typically needs time to recover.
*
* @return   None
*
* @note     None
*
******************************************************************************/
void EmacLitePhyDelay(unsigned int Seconds)
{
#ifdef __MICROBLAZE__
	static int WarningFlag = 0;

	/* If MB caches are disabled or do not exist, this delay loop could
	 * take minutes instead of seconds (e.g., 30x longer).  Print a warning
	 * message for the user (once).  If only MB had a built-in timer!
	 */
	if (((mfmsr() & 0x20) == 0) && (!WarningFlag)) {
#ifdef STDOUT_BASEADDRESS
		xil_printf("Warning: This example will take ");
		xil_printf("minutes to complete without I-cache enabled \r\n");
#endif
		WarningFlag = 1;
	}

#define ITERS_PER_SEC   (XPAR_CPU_CORE_CLOCK_FREQ_HZ / 6)
    asm volatile ("\n"
                  "1:               \n\t"
                  "addik r7, r0, %0 \n\t"
                  "2:               \n\t"
                  "addik r7, r7, -1 \n\t"
                  "bneid  r7, 2b    \n\t"
                  "or  r0, r0, r0   \n\t"
                  "bneid %1, 1b     \n\t"
                  "addik %1, %1, -1 \n\t"
                  :: "i"(ITERS_PER_SEC), "d" (Seconds));

#else

//	usleep(Seconds * 1000000);
//	waitfor(Seconds * 1000);
#endif
}


