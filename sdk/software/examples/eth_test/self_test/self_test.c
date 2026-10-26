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




/*
 * The following constants map to the XPAR parameters created in the
 * xparameters.h file. They are defined here such that a user can easily
 * change all the needed parameters in one place.
 */
#define EMAC_DEVICE_ID			240
#define ETH_BASE_REG 0x44000
volatile unsigned int* eth_base_reg = ETH_BASE_REG;

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

int EMACLiteSelfTestExample(u16 DeviceId);
extern XEmacLite_Config XEmacLite_ConfigTable;

/************************** Variable Definitions *****************************/

/*
 * Instance of the driver
 */
static XEmacLite EmacLite;

/*****************************************************************************/
/**
*
* Performs a SelfTest on the EmacLite device as follows:
*   - Writes to the mandatory TX buffer and reads back to verify.
*   - If configured, writes to the secondary TX buffer and reads back to verify.
*   - Writes to the mandatory RX buffer and reads back to verify.
*   - If configured, writes to the secondary RX buffer and reads back to verify.
*
*
* @param	InstancePtr is a pointer to the XEmacLite instance .
*
* @return
*		- SUCCESS if the device Passed the Self Test.
* 		- FAILURE if any of the data read backs fail.
*
* @note		None.
*
******************************************************************************/
int XEmacLite_SelfTest(XEmacLite * InstancePtr)
{
	UINTPTR BaseAddress;
	volatile uint32_t * TempAddress;
	u8 Index;
#if 0
	u8 TestString[4] = { 0xDE, 0xAD, 0xBE, 0xEF };
	u8 ReturnString[4] = { 0x0, 0x0, 0x0, 0x0 };
#else
	u32 TestString[4] = { 0xDEADBEEF, 0xABCDABCD, 0xAABBCCDD, 0x12345678 };
	u32 ReturnString[4] = { 0x0, 0x0, 0x0, 0x0 };

#endif

	/*
	 * Verify that each of the inputs are valid.
	 */
	if (InstancePtr == NULL) {
		return -1;
	}

	/*
	 * Determine the TX buffer address
	 */
	BaseAddress = InstancePtr->EmacLiteConfig.BaseAddress +
			XEL_TXBUFF_OFFSET;
#if 0
	log_info("\n Base Address is %x",  BaseAddress);
	TempAddress = (volatile uint32_t *)BaseAddress;
	*TempAddress = 0x12345678;
	*(TempAddress++) = 0x9abcdef;
	*(TempAddress++) = 0x11223344;
	*(TempAddress++) = 0x55667788;
	*(TempAddress++) = 0x99aabbcc;
	*(TempAddress++) = 0xddeeff00;

	TempAddress = (volatile uint32_t *)BaseAddress;
	printf("\n Value at %x: %x", TempAddress , *TempAddress );
	printf("\n Value at %x: %x", (TempAddress) , *TempAddress++ );
	printf("\n Value at %x: %x", TempAddress , *TempAddress++ );
	printf("\n Value at %x: %x", (TempAddress) , *TempAddress++ );
	printf("\n Value at %x: %x", TempAddress , *TempAddress++ );
	printf("\n Value at %x: %x", (TempAddress) , *TempAddress++ );
	printf("\n Value at %x: %x", TempAddress , *TempAddress++ );
	printf("\n Value at %x: %x", (TempAddress) , *TempAddress++ );

	BaseAddress = InstancePtr->EmacLiteConfig.BaseAddress +
			(XEL_TXBUFF_OFFSET + 4);
#endif
	/*
	 * Write the TestString to the TX buffer in EMAC Lite then
	 * back from the EMAC Lite and verify
	 */
	XEmacLite_AlignedWrite(TestString, (UINTPTR *) BaseAddress,
			       sizeof(TestString));
	XEmacLite_AlignedRead((UINTPTR *) BaseAddress, ReturnString,
			      sizeof(ReturnString));
	for (Index = 0; Index < 4; Index++) {

		printf("\n TestString[%x]: %x; ReturnString[%x]: %x", Index, TestString[Index], Index, ReturnString[Index]);
		if (ReturnString[Index] != TestString[Index]) {
			return FAILURE;
		}

		/*
		 * Zero the return string for the next test
		 */
		ReturnString[Index] = 0;
	}

	/*
	 * If the second buffer is configured, test it also
	 */
	if (InstancePtr->EmacLiteConfig.TxPingPong != 0) {
		BaseAddress += XEL_BUFFER_OFFSET;
		/*
		 * Write the TestString to the optional TX buffer in EMAC Lite
		 * then back from the EMAC Lite and verify
		 */
		XEmacLite_AlignedWrite(TestString, (UINTPTR *) BaseAddress,
				       sizeof(TestString));
		XEmacLite_AlignedRead((UINTPTR *) BaseAddress, ReturnString,
				      sizeof(ReturnString));

		for (Index = 0; Index < 4; Index++) {

			if (ReturnString[Index] != TestString[Index]) {
				return FAILURE;
			}

			/*
			 * Zero the return string for the next test
			 */
			ReturnString[Index] = 0;
		}
	}

	/*
	 * Determine the RX buffer address
	 */
	BaseAddress = InstancePtr->EmacLiteConfig.BaseAddress +
				XEL_RXBUFF_OFFSET;

	/*
	 * Write the TestString to the RX buffer in EMAC Lite then
	 * back from the EMAC Lite and verify
	 */
	XEmacLite_AlignedWrite(TestString, (UINTPTR *) (BaseAddress),
			       sizeof(TestString));
	XEmacLite_AlignedRead((UINTPTR *) (BaseAddress), ReturnString,
			      sizeof(ReturnString));

	for (Index = 0; Index < 4; Index++) {

		if (ReturnString[Index] != TestString[Index]) {
			return FAILURE;
		}

		/*
		 * Zero the return string for the next test
		 */
		ReturnString[Index] = 0;
	}

	/*
	 * If the second buffer is configured, test it also
	 */
	if (InstancePtr->EmacLiteConfig.RxPingPong != 0) {
		BaseAddress += XEL_BUFFER_OFFSET;
		/*
		 * Write the TestString to the optional RX buffer in EMAC Lite
		 * then back from the EMAC Lite and verify
		 */
		XEmacLite_AlignedWrite(TestString, (UINTPTR *) BaseAddress,
				       sizeof(TestString));
		XEmacLite_AlignedRead((UINTPTR *) BaseAddress, ReturnString,
				      sizeof(ReturnString));

		for (Index = 0; Index < 4; Index++) {

			if (ReturnString[Index] != TestString[Index]) {
				return FAILURE;
			}

			/*
			 * Zero the return string for the next test
			 */
			ReturnString[Index] = 0;
		}
	}

	return SUCCESS;
}



/****************************************************************************/
/**
*
* This function is the main function of the EmacLite selftest example.
*
* @param	None
*
* @return	SUCCESS to indicate success, else FAILURE.
*
* @note		None
*
*****************************************************************************/
int main(void)
{
	int Status;

	/*
	 * Run the EmacLite Self test example, specify the Device ID that is
	 * generated in xparameters.h
	 */
	Status = EMACLiteSelfTestExample(EMAC_DEVICE_ID);
	if (Status != SUCCESS) {
		log_error("Emaclite selftest Example Failed\r\n");
		return FAILURE;
	}

	log_debug("Successfully ran Emaclite selftest Example\r\n");
	return SUCCESS;

}

/*****************************************************************************/
/**
*
* The main entry point for the EmacLite driver selftest example.
*
* @param	DeviceId is the XPAR_<xemaclite_instance>_DEVICE_ID value from
*		xparameters.h
*
* @return	SUCCESS to indicate success, else FAILURE.
*
* @note		None.
*
******************************************************************************/
int EMACLiteSelfTestExample(u16 DeviceId)
{
	int Status;
	XEmacLite_Config *ConfigPtr;
	XEmacLite *InstancePtr = &EmacLite;
	log_debug("\n Inside EMACLiteSelfTestExample");
	/*
	 * Initialize the EmacLite device.
	 */
	ConfigPtr = &XEmacLite_ConfigTable;
	if (ConfigPtr == NULL) {
		return -1;
	}
	ConfigPtr->BaseAddress = (UINTPTR *) 0X44000;
	log_debug("\n ConfigPtr: BaseAddress: %x", ConfigPtr->BaseAddress);
	log_debug("\n ConfigPtr: DeviceId: %x", ConfigPtr->DeviceId);
	log_debug("\n ConfigPtr: TxPingPong: %x", ConfigPtr->TxPingPong);
	log_debug("\n ConfigPtr: RxPingPong: %x", ConfigPtr->RxPingPong);
	log_debug("\n ConfigPtr: MdioInclude: %x", ConfigPtr->MdioInclude);
	log_debug("\n ConfigPtr: Loopback: %x", ConfigPtr->Loopback);
	Status = XEmacLite_CfgInitialize(InstancePtr,
					ConfigPtr,
					ConfigPtr->BaseAddress);
	if (Status != 0) {
		return -1;
	}
	
	/*
	 * Run the Self Test
	 */
	log_debug("\n Calling self test function.");
	Status = XEmacLite_SelfTest(InstancePtr);
	if (Status != SUCCESS) {
		return FAILURE;
	}

	return SUCCESS;
}

