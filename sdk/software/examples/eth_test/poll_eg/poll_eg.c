/***************************************************************************
 * Project           			:  shakti devt board
 * Name of the file	     		:  btnled.c
 * Brief Description of file    :  Controls the led operation with help of button,gpio based.
 * Name of Author    	        :  Sathya Narayanan N & Abhinav Ramnath
 * Email ID                     :  sathya281@gmail.com  & abhinavramnath13@gmail.com 

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
@file pwminterrupt
@brief A application to demonstrate working of pwm interrupts
@detail This file contains an application to demonstrate the working of pwm
 interrupts. The interrupts are enabled for a pwm module. The pwm module is
 confiured for interrupt mode and pwm registers are setup appropriately.
 Once the pwm timer overflows, an interrupt is generated and it is handled
 by the isr.
*/

#include "gpio.h"
#include "uart.h"
#include "traps.h"
#include "platform.h"
#include "eth_driver.h"
#include "log.h"
#include "poll_eg.h"



/*
 * The following constants map to the XPAR parameters created in the
 * xparameters.h file. They are defined here such that a user can easily
 * change all the needed parameters in one place.
 */
#define EMAC_DEVICE_ID			240
#define ETH_BASE_REG 0x44000
volatile unsigned int* eth_base_reg = ETH_BASE_REG;
/*
 * The Size of the Test Frame.
 */
#define EMACLITE_TEST_FRAME_SIZE	1000

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

int EmacLitePolledExample(u16 DeviceId);

static int EmacLiteSendFrame(XEmacLite *InstancePtr, u32 PayloadSize);

static int EmacLiteRecvFrame(u32 PayloadSize);

extern u16 Xil_EndianSwap16(u16 Data);

/************************** Variable Definitions *****************************/

/*
 * Set up valid local and remote MAC addresses. This loop back test uses the
 * LocalAddress both as a source and destination MAC address.
 */
static u8 LocalAddress[XEL_MAC_ADDR_SIZE] =
{
	0x00, 0x0A, 0x35, 0x01, 0x02, 0x03
};
static u8 RemoteAddress[XEL_MAC_ADDR_SIZE] =
{
	0x00, 0x10, 0xa4, 0xb6, 0xfd, 0x09
};

extern XEmacLite_Config XEmacLite_ConfigTable;

/************************** Variable Definitions *****************************/

/*
 * Instance of the driver
 */
static XEmacLite EmacLite;

/****************************************************************************/
/**
*
* This function is the main function of the EmacLite polled example.
*
* @param	None.
*
* @return	SUCCESS to indicate success, otherwise FAILURE .
*
* @note		None.
*
*****************************************************************************/
int main()
{
	int Status;

	/*
	 * Run the EmacLite Polled example, specify the Device ID that is
	 * generated in xparameters.h.
	 */
	Status = EmacLitePolledExample(EMAC_DEVICE_ID);
	if (Status != SUCCESS) {
		log_debug("Emaclite polled Example Failed\r\n");
		return FAILURE;
	}

	log_debug("Successfully ran Emaclite polled Example\r\n");
	return SUCCESS;
}


/*****************************************************************************/
/**
*
* The main entry point for the EmacLite driver example in polled mode.
*
* This function will transmit/receive the Ethernet frames and verify the
* data in the received frame (if the MDIO interface is configured in the
* EmacLite core).
* This function simply transmits a frame if the MDIO interface is not
* configured in the EmacLite core.
*
* @param	DeviceId is device ID of the XEmacLite Device , typically
*		XPAR_<EMAC_instance>_DEVICE_ID value from xparameters.h.
*
* @return	SUCCESS to indicate success, FAILURE otherwise.
*
* @note		None.
*
******************************************************************************/
int EmacLitePolledExample(u16 DeviceId)
{
	int Status;
	XEmacLite *EmacLiteInstPtr = &EmacLiteInstance;
	u32 PhyAddress = 0;
	RecvFrameLength = 0;
	XEmacLite_Config *ConfigPtr;

	ConfigPtr = &XEmacLite_ConfigTable;
	if (ConfigPtr == NULL) {
		return -1;
	}
	ConfigPtr->BaseAddress = (UINTPTR *) 0X44000;
	ConfigPtr->RxPingPong = XPAR_EMACLITE_0_RX_PING_PONG;
	ConfigPtr->MdioInclude = XPAR_EMACLITE_0_INCLUDE_MDIO;
	ConfigPtr->TxPingPong = XPAR_EMACLITE_0_TX_PING_PONG;
	log_debug("\n ConfigPtr: BaseAddress: %x", ConfigPtr->BaseAddress);
	log_debug("\n ConfigPtr: DeviceId: %x", ConfigPtr->DeviceId);
	log_debug("\n ConfigPtr: TxPingPong: %x", ConfigPtr->TxPingPong);
	log_debug("\n ConfigPtr: RxPingPong: %x", ConfigPtr->RxPingPong);
	log_debug("\n ConfigPtr: MdioInclude: %x", ConfigPtr->MdioInclude);
	log_debug("\n ConfigPtr: Loopback: %x", ConfigPtr->Loopback);
	Status = XEmacLite_CfgInitialize(EmacLiteInstPtr,
					ConfigPtr,
					ConfigPtr->BaseAddress);
	if (Status != 0) {
		return -1;
	}

	/*
	 * Set the MAC address.
	 */
	XEmacLite_SetMacAddress(EmacLiteInstPtr, LocalAddress);

	/*
	 * Check if there is a TX buffer available, if there isn't it is an
	 * error.
	 */
	if (XEmacLite_TxBufferAvailable(EmacLiteInstPtr) != TRUE) {
		return FAILURE;
	}

	/*
	 * If the MDIO is configured in the device.
	 */
	if (XEmacLite_IsMdioConfigured(EmacLiteInstPtr)) {
		/*
		 * Detect the PHY device and enable the MAC Loop back
		 * in the PHY.
		 */
		PhyAddress = EmacLitePhyDetect(EmacLiteInstPtr);
		Status = EmacLiteEnablePhyLoopBack(EmacLiteInstPtr,
							 PhyAddress);
		if (Status != SUCCESS) {
			return FAILURE;
		}
	}

	/*
	 * Empty any existing receive frames.
	 */
	XEmacLite_FlushReceive(EmacLiteInstPtr);

	/*
	 * Reset the receive frame length to zero.
	 */
	RecvFrameLength = 0;
	Status = EmacLiteSendFrame(EmacLiteInstPtr, EMACLITE_TEST_FRAME_SIZE);
	if (Status != SUCCESS) {
		if (XEmacLite_IsMdioConfigured(EmacLiteInstPtr)) {
			/*
			 * Disable the MAC Loop back in the PHY.
			 */
			EmacLiteDisablePhyLoopBack(EmacLiteInstPtr,
							 PhyAddress);
			return FAILURE;
		}
	}

	/*
	 * If the MDIO is not configured in the core then return SUCCESS
	 * as the frame has been transmitted.
	 */
	if (!XEmacLite_IsMdioConfigured(EmacLiteInstPtr)) {
		return SUCCESS;
	}


	/*
	 * Poll for receive packet.
	 */
	while ((volatile u32)RecvFrameLength == 0)  {
		RecvFrameLength = XEmacLite_Recv(EmacLiteInstPtr,
						(u8 *)RxFrame);
	}

	/*
	 * Check the received frame.
	 */
	Status = EmacLiteRecvFrame(EMACLITE_TEST_FRAME_SIZE);
	if ((Status != SUCCESS) && (Status != NO_DATA)) {
		/*
		 * Disable the MAC Loop back in the PHY.
		 */
		EmacLiteDisablePhyLoopBack(EmacLiteInstPtr, PhyAddress);
		return FAILURE;
	}


	/*
	 * Disable the MAC Loop back in the PHY.
	 */
	EmacLiteDisablePhyLoopBack(EmacLiteInstPtr, PhyAddress);

	return SUCCESS;
}

/******************************************************************************/
/**
*
* This function sends a frame of given size.
*
* @param	XEmacInstancePtr is a pointer to the XEmacLite instance.
* @param	PayloadSize is the size of the frame to create. The size only
*		reflects the payload size, it does not include the Ethernet
*		header size (14 bytes) nor the Ethernet CRC size (4 bytes).
*
* @return	SUCCESS if successful, else a driver-specific return code.
*
* @note		None.
*
******************************************************************************/
static int EmacLiteSendFrame(XEmacLite *InstancePtr, u32 PayloadSize)
{
	u8 *FramePtr;
	int Index;
	FramePtr = (u8 *)TxFrame;

	/*
	 * Set up the destination address as the local address for
	 * Phy Loopback.
	 */
	if (XEmacLite_IsMdioConfigured(InstancePtr)) {

		*FramePtr++ = LocalAddress[0];
		*FramePtr++ = LocalAddress[1];
		*FramePtr++ = LocalAddress[2];
		*FramePtr++ = LocalAddress[3];
		*FramePtr++ = LocalAddress[4];
		*FramePtr++ = LocalAddress[5];
	} else {
		/*
		 * Fill in the valid Destination MAC address if
		 * the Loopback is not enabled.
		 */
		*FramePtr++ = RemoteAddress[0];
		*FramePtr++ = RemoteAddress[1];
		*FramePtr++ = RemoteAddress[2];
		*FramePtr++ = RemoteAddress[3];
		*FramePtr++ = RemoteAddress[4];
		*FramePtr++ = RemoteAddress[5];
	}

	/*
	 * Fill in the source MAC address.
	 */
	*FramePtr++ = LocalAddress[0];
	*FramePtr++ = LocalAddress[1];
	*FramePtr++ = LocalAddress[2];
	*FramePtr++ = LocalAddress[3];
	*FramePtr++ = LocalAddress[4];
	*FramePtr++ = LocalAddress[5];

	/*
	 * Set up the type/length field - be sure its in network order.
	 */
    *((u16 *)FramePtr) = Xil_EndianSwap16(PayloadSize);
	FramePtr++;
	FramePtr++;

	/*
	 * Now fill in the data field with known values so we can verify them
	 * on receive.
	 */
	for (Index = 0; Index < PayloadSize; Index++) {
		*FramePtr++ = (u8)Index;
	}

	/*
	 * Now send the frame.
	 */
	return XEmacLite_Send(InstancePtr, (u8 *)TxFrame,
				PayloadSize + XEL_HEADER_SIZE);

}

/******************************************************************************/
/**
*
* This function receives a frame of given size. This function assumes interrupt
* mode, receives the frame and verifies its contents.
*
* @param	PayloadSize is the size of the frame to receive.
*		The size only reflects the payload size, it does not include the
*		Ethernet header size (14 bytes) nor the Ethernet CRC size (4
*		bytes).
*
* @return	SUCCESS if successful, a driver-specific return code if not.
*
* @note		None.
*
******************************************************************************/
static int EmacLiteRecvFrame(u32 PayloadSize)
{
	u8 *FramePtr;

	/*
	 * This assumes MAC does not strip padding or CRC.
	 */
	if (RecvFrameLength != 0) {
		int Index;

		/*
		 * Verify length, which should be the payload size.
		 */
		if ((RecvFrameLength- (XEL_HEADER_SIZE + XEL_FCS_SIZE)) !=
				PayloadSize) {
			return XST_LOOPBACK_ERROR;
		}

		/*
		 * Verify the contents of the Received Frame.
		 */
		FramePtr = (u8 *)RxFrame;
		FramePtr += XEL_HEADER_SIZE;	/* Get past the header */

		for (Index = 0; Index < PayloadSize; Index++) {
			if (*FramePtr++ != (u8)Index) {
				return XST_LOOPBACK_ERROR;
			}
		}
	}

	return SUCCESS;
}
