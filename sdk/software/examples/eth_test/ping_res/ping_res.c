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
/*
 * Instance of the driver
 */
static XEmacLite EmacLite;



/*
 * Change this parameter to limit the number of ping replies sent by this
 * program.
 */
#define MAX_PING_REPLIES	10000	/* Maximum number of ping replies */

#define BROADCAST_PACKET	1	/* Broadcast packet */
#define MAC_MATCHED_PACKET	2 	/* Dest MAC matched with local MAC */
#define IP_ADDR_SIZE		4	/* IP Address size in Bytes */
#define ARP_REQUEST 		0x0001	/* ARP Request bits in Rx packet */
#define ARP_REPLY 		0x0002 	/* ARP status bits indicating reply */
#define ARP_PACKET_SIZE 	0x3C    /* ARP packet len 60 Bytes */
#define ICMP_PACKET_SIZE 	98 //0x4A    /* ICMP packet length 74 Bytes
					 /* including Src and Dest MAC Address */
#define BROADCAST_ADDR 		0xFFFF  /* Broadcast Address */
#define CORRECT_CKSUM_VALUE	0xFFFF  /* Correct checksum value */
#define IDENT_FIELD_VALUE	0x9263	/* Identification field (random num) */

/*
 * Definitions for the locations and length of some of the fields in a
 * IP packet. The lengths are defined in Half-Words (2 bytes).
 */
#define ETHER_PROTO_TYPE_LEN	1  /* Ethernet protocol Type length  */
#define SRC_MAC_ADDR_LOC	3  /* Source MAC address location */
#define MAC_ADDR_LEN 		3  /* MAC address length */
#define ETHER_PROTO_TYPE_LOC	6  /* Ethernet Proto type location */

#define ARP_HW_TYPE_LEN 	1  /* Hardware Type length  */
#define ARP_PROTO_TYPE_LEN	1  /* Protocol Type length  */
#define ARP_HW_ADD_LEN		1  /* Hardware address length */
#define ARP_PROTO_ADD_LEN	1  /* Protocol address length */
#define ARP_ZEROS_LEN		9  /* Length to be filled with zeros */
#define ARP_REQ_STATUS_LOC 	10 /* ARP request location */
#define ARP_REQ_SRC_IP_LOC 	14 /* Src IP address location of ARP request */
#define ARP_REQ_DEST_IP_LOC_1 	19 /* Destination IP's 1st half word location */
#define ARP_REQ_DEST_IP_LOC_2 	20 /* Destination IP's 2nd half word location */

#define IP_VERSION_LEN 		1  /* IP Version length  */
#define IP_PACKET_LEN 		1  /* IP Packet length field  */
#define IP_FRAG_FIELD_LEN	1  /* Fragment field len in ICMP packet */
#define IP_TTL_ICM_LEN 		1  /* Time to live and ICM fields length */
#define IP_ADDR_LEN 		2  /* Size of IP address in half-words */
#define IP_CSUM_LOC_BACK	5  /* IP checksum location from end of frame */
#define IP_HDR_START_LOC 	7  /* IP header start location */
#define IP_HDR_LEN 		10 /* IP Header length */
#define IP_FRAG_FIELD_LOC 	10 /* Fragment field location */

#define ICMP_TYPE_LEN 		1  /* ICMP Type length */
#define ICMP_ECHO_FIELD_LEN 	2  /* Echo field length in half-words */
#define ICMP_REQ_SRC_IP_LOC 	13 /* Src IP address location of ICMP request */
#define ICMP_ECHO_FIELD_LOC 	17 /* Echo field location */
#define ICMP_DATA_START_LOC 	17 /* Data field start location */
#define ICMP_DATA_LEN 		30  //18  /* ICMP data length */
#define ICMP_DATA_LOC 		19 /* ICMP data location including
				      identifier number and sequence number */
#define ICMP_DATA_CSUM_LOC_BACK 31 //19 
/* Data checksum location from end of
					frame */
#define ICMP_DATA_FIELD_LEN 	32 //20 /* Data field length */

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

static int EmacLitePingReplyExample(u16 DeviceId);

static void ProcessRecvFrame(XEmacLite *InstancePtr);

static u16 CheckSumCalculation(u16 *RxFramePtr, int StartLoc, int Length);

/************************** Variable Definitions *****************************/

/*
 * Set up a local MAC address.
 */
static u8 LocalMacAddr[XEL_MAC_ADDR_SIZE] =
{
	0x00, 0x00, 0x5E, 0x00, 0xFA, 0xCE
};

/*
 * The IP address was set to 172.16.63.121. User need to set a free IP address
 * based on the network on which this example is to be run.
 */
static u8 LocalIpAddr[IP_ADDR_SIZE] =
{
	192, 168, 1, 25
};

static XEmacLite EmacLiteInstance;	/* Instance of the EmacLite driver */

/*
 * Buffers used for Transmission and Reception of Packets. These are declared as
 * global so that they are not a part of the stack.
 */
static u8 RxFrame[XEL_MAX_FRAME_SIZE];
static u8 TxFrame[XEL_MAX_FRAME_SIZE];

/*
 * Variable used to indicate the length of the received frame.
 */
extern volatile u32 RecvFrameLength = 0;

/*
 * Variable used to indicate the number of Ping replies sent.
 */
u32 NumOfPingReplies;
u32 arpcount = 0;

/****************************************************************************/
/**
*
* This function is the main function of the Ping reply example in
* polled mode.
*
* @param	None.
*
* @return	XST_FAILURE to indicate failure, otherwise XST_SUCCESS
*		is returned.
*
* @note		None.
*
*****************************************************************************/
int main()
{
	int Status;
	arpcount = 0;
	/*
	 * Run the EmacLite Ping reply example.
	 */
	Status = EmacLitePingReplyExample(EMAC_DEVICE_ID);
	if (Status != XST_SUCCESS) {
		xil_printf("Emaclite ping reply Example Failed\r\n");
		return XST_FAILURE;
	}

	xil_printf("Successfully ran Emaclite ping reply Example\r\n");
	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* The entry point for the EmacLite Ping reply example in polled mode.
*
* @param	DeviceId is device ID of the XEmacLite Device.
*
* @return	XST_FAILURE to indicate failure, otherwise XST_SUCCESS is
*		returned.
*
* @note		This is in a continuous loop generating a specified number of
*		ping replies as defined by MAX_PING_REPLIES.
*
******************************************************************************/
int EmacLitePingReplyExample(u16 DeviceId)
{
	int Status, count = 0;
	XEmacLite *EmacLiteInstPtr = &EmacLiteInstance;
	XEmacLite_Config *ConfigPtr;
	NumOfPingReplies = 0;

	/*
	 * Initialize the EmacLite device.
	 */
	ConfigPtr = &XEmacLite_ConfigTable;
	if (ConfigPtr == NULL) {
		return -1;
	}
	printf("\n ");

	ConfigPtr->BaseAddress = (UINTPTR *) 0X44000;
	ConfigPtr->RxPingPong = XPAR_EMACLITE_0_RX_PING_PONG;
	ConfigPtr->MdioInclude = XPAR_EMACLITE_0_INCLUDE_MDIO;
	ConfigPtr->TxPingPong = XPAR_EMACLITE_0_TX_PING_PONG;
	Status = XEmacLite_CfgInitialize(EmacLiteInstPtr,
					ConfigPtr,
					ConfigPtr->BaseAddress);
	if (Status != 0) {
		return -1;
	}
	
	/*
	 * Set the MAC address.
	 */
	XEmacLite_SetMacAddress(EmacLiteInstPtr, LocalMacAddr);
	/*
	 * Empty any existing receive frames.
	 */
	XEmacLite_FlushReceive(EmacLiteInstPtr);

	while (1) {

		/*
		 * Wait for a Receive packet.
		 */
		while (RecvFrameLength == 0) {
			RecvFrameLength = XEmacLite_Recv(EmacLiteInstPtr,
								(u8 *)RxFrame);
		}
#if 1
		printf("\n Rcv. Frame %x: %x\n", NumOfPingReplies, RecvFrameLength);
		for(int i = 0; i < RecvFrameLength; i++)
			printf("%x ", RxFrame[i]);
		printf("\n");
#endif			
		/*
		 * Empty any existing receive frames.
		 */
		XEmacLite_FlushReceive(EmacLiteInstPtr);

		/*
		 * Process the Receive frame.
		 */
		ProcessRecvFrame(EmacLiteInstPtr);
		RecvFrameLength = 0;

		/*
		 * If the number of ping replies sent is equal to that
		 * specified by the user then exit out of this loop.
		 */
		if (NumOfPingReplies == MAX_PING_REPLIES) {

			return XST_SUCCESS;
		}

	}
}

/******************************************************************************/
/**
*
* This function processes the received packet and generates the corresponding
* reply packets.
*
* @param	InstancePtr is a pointer to the instance of the EmacLite.
*
* @return	None.
*
* @note		This function assumes MAC does not strip padding or CRC.
*
******************************************************************************/
static void ProcessRecvFrame(XEmacLite *InstancePtr)
{
	u16 *RxFramePtr;
	u16 *TxFramePtr;
	u16 *TempPtr;
	u16 CheckSum;
	u32 NextTxBuffBaseAddr;
	int Index;
	int PacketType = 0;

	TxFramePtr = (u16 *)TxFrame;
	RxFramePtr = (u16 *)RxFrame;

	/*
	 * Determine the next expected Tx buffer address.
	 */
	NextTxBuffBaseAddr = XEmacLite_NextTransmitAddr(InstancePtr);

	/*
	 * Check the packet type.
	 */
	Index = MAC_ADDR_LEN;
	TempPtr = (u16 *)LocalMacAddr;
	while (Index--) {
		if (Xil_Ntohs((*(RxFramePtr + Index)) == BROADCAST_ADDR) &&
					(PacketType != MAC_MATCHED_PACKET)) {
			PacketType = BROADCAST_PACKET;
		} else if (Xil_Ntohs((*(RxFramePtr + Index)) == *(TempPtr + Index)) &&
					(PacketType != BROADCAST_PACKET)) {
			PacketType = MAC_MATCHED_PACKET;
		} else {
			PacketType = 0;
			break;
		}
	}
#if 0
	printf("\n Packet type is %x", PacketType);
#endif
	/*
	 * Process broadcast packet.
	 */
	if (PacketType == BROADCAST_PACKET) {

		/*
		 * Check for an ARP Packet if so generate a reply.
		 */
		if (Xil_Ntohs(*(RxFramePtr + ETHER_PROTO_TYPE_LOC)) ==
				XEL_ETHER_PROTO_TYPE_ARP) {

			/*
			 * IP address of the local machine.
			 */
			TempPtr = (u16 *)LocalIpAddr;

			/*
			 * Check destination IP address of the packet with
			 * local IP address.
			 */
			if (
			((*(RxFramePtr + ARP_REQ_DEST_IP_LOC_1)) == *TempPtr++) &&
			((*(RxFramePtr + ARP_REQ_DEST_IP_LOC_2)) == *TempPtr++)) {

				/*
				 * Check ARP packet type(request/reply).
				 */
				if (Xil_Ntohs(*(RxFramePtr + ARP_REQ_STATUS_LOC)) ==
								ARP_REQUEST) {

					/*
					 * Add destination MAC address
					 * to the reply packet (i.e) source
					 * address of the received packet.
					 */
					Index = SRC_MAC_ADDR_LOC;
					while (Index < (SRC_MAC_ADDR_LOC +
							MAC_ADDR_LEN)) {
						*TxFramePtr++ =
							*(RxFramePtr + Index);
						Index++;
					}

					/*
					 * Add source (local) MAC address
					 * to the reply packet.
					 */
					Index = 0;
					TempPtr = (u16 *)LocalMacAddr;
					while (Index < MAC_ADDR_LEN) {
						*TxFramePtr++ = *TempPtr++;
						Index++;
					}

					/*
					 * Add Ethernet proto type H/W
					 * type(10/3MBps),H/W address length and
					 * protocol address len (i.e)same as in
					 * the received packet
					 */
					Index = ETHER_PROTO_TYPE_LOC;
					while (Index < (ETHER_PROTO_TYPE_LOC +
							ETHER_PROTO_TYPE_LEN +
							ARP_HW_TYPE_LEN +
							ARP_HW_ADD_LEN
							+ ARP_PROTO_ADD_LEN)) {
						*TxFramePtr++ =
							*(RxFramePtr + Index);
						Index++;
					}

					/*
					 * Add ARP reply status to the reply
					 * packet.
					 */
					*TxFramePtr++ = Xil_Htons(ARP_REPLY);

					/*
					 * Add local MAC Address
					 * to the reply packet.
					 */
					TempPtr = (u16 *)LocalMacAddr;
					Index = 0;
					while (Index < MAC_ADDR_LEN) {
						*TxFramePtr++ = *TempPtr++;
						Index++;
					}

					/*
					 * Add local IP Address
					 * to the reply packet.
					 */
					TempPtr = (u16 *)LocalIpAddr;
					Index = 0;
					while (Index < IP_ADDR_LEN) {
						*TxFramePtr++ = *TempPtr++ ;
						Index++;
					}

					/*
					 * Add Destination MAC Address
					 * to the reply packet from the received
					 * packet.
					 */
					Index = SRC_MAC_ADDR_LOC;
					while (Index < (SRC_MAC_ADDR_LOC +
							MAC_ADDR_LEN)) {
						*TxFramePtr++ =
							*(RxFramePtr + Index);
						Index++;
					}

					/*
					 * Add Destination IP Address
					 * to the reply packet.
					 */
					Index = ARP_REQ_SRC_IP_LOC;
					while (Index < (ARP_REQ_SRC_IP_LOC +
							IP_ADDR_LEN)) {
						*TxFramePtr++ =
								*(RxFramePtr + Index);
						Index++;
					}

					/*
					 * Fill zeros as per protocol.
					 */
					Index = 0;
					while (Index < ARP_ZEROS_LEN) {
						*TxFramePtr++ = 0x0000;
						Index++;
					}

					/*
					 * Transmit the Reply Packet.
					 */
					XEmacLite_Send(InstancePtr,
							(u8 *)&TxFrame,
							ARP_PACKET_SIZE);
				}
			}
		}
	}

	/*
	 * Process packets whose MAC address is matched.
	 */
	if (PacketType == MAC_MATCHED_PACKET) {
//		printf("\n MAC matched packet");
		/*
		 * Check ICMP packet.
		 */
		if (Xil_Ntohs(*(RxFramePtr + ETHER_PROTO_TYPE_LOC)) ==
						XEL_ETHER_PROTO_TYPE_IP) {
//			printf("\n calculate ip header checksum");
			/*
			 * Check the IP header checksum.
			 */
			CheckSum = CheckSumCalculation(RxFramePtr,
						IP_HDR_START_LOC,
						IP_HDR_LEN);

			/*
			 * Check the Data field checksum.
			 */
//			printf("\n ip checksum:%x", CheckSum);
			if (CheckSum == CORRECT_CKSUM_VALUE) {
				CheckSum = CheckSumCalculation(RxFramePtr,
						ICMP_DATA_START_LOC,
						ICMP_DATA_FIELD_LEN);
#if 0
			printf("\nICMP Rcv Packet: \n");
			for(int i = ICMP_DATA_START_LOC; i <= ICMP_DATA_FIELD_LEN; i++)
			   printf("%x ", RxFramePtr[i]);
			printf("\n End of ICMP Rcv Packet");
				
				printf("\n data checksum:%x", CheckSum);
#endif
				if (CheckSum == CORRECT_CKSUM_VALUE) {

					/*
					 * Add destination address
					 * to the reply packet (i.e)source
					 * address of the received packet.
					 */
					Index = SRC_MAC_ADDR_LOC;
					while (Index < (SRC_MAC_ADDR_LOC +
							MAC_ADDR_LEN)) {
						*TxFramePtr++ =
							*(RxFramePtr + Index);
						Index++;
					}

					/*
					 * Add local MAC address
					 * to the reply packet.
					 */
					Index = 0;
					TempPtr = (u16 *)LocalMacAddr;
					while (Index < MAC_ADDR_LEN) {
						*TxFramePtr++ = *TempPtr++;
						Index++;
					}

					/*
					 * Add protocol type
					 * header length and, packet
					 * length(60 Bytes) to the reply packet.
					 */
					Index = ETHER_PROTO_TYPE_LOC;
					while (Index < (ETHER_PROTO_TYPE_LOC +
							ETHER_PROTO_TYPE_LEN +
							IP_VERSION_LEN +
							IP_PACKET_LEN)) {
						*TxFramePtr++ =
							*(RxFramePtr + Index);
						Index++;
					}

					/*
					 * Identification field a random number
					 * which is set to IDENT_FIELD_VALUE.
					 */
					*TxFramePtr++ = IDENT_FIELD_VALUE;

					/*
					 * Add fragment type, time to live and
					 * ICM field. It is same as in the
					 * received packet.
					 */
					Index = IP_FRAG_FIELD_LOC;
					while (Index < (IP_FRAG_FIELD_LOC +
							IP_TTL_ICM_LEN +
							IP_FRAG_FIELD_LEN)) {
						*TxFramePtr++ =
							*(RxFramePtr + Index);
						Index++;
					}

					/*
					 * Checksum first set to 0 and
					 * added in this field later.
					 */
					*TxFramePtr++ = 0x0000;

					/*
					 * Add Source IP address
					 */
					Index = 0;
					TempPtr = (u16 *)LocalIpAddr;
					while (Index < IP_ADDR_LEN) {
						*TxFramePtr++ = *TempPtr++;
						Index++;
					}

					/*
					 * Add Destination IP address.
					 */
					Index = ICMP_REQ_SRC_IP_LOC;
					while (Index < (ICMP_REQ_SRC_IP_LOC +
							IP_ADDR_LEN)) {
						*TxFramePtr++ =
							*(RxFramePtr + Index);
						Index++;
					}

					/*
					 * Calculate checksum, and
					 * add it in the appropriate field.
					 */
					CheckSum = CheckSumCalculation(
							(u16 *)TxFrame,
							IP_HDR_START_LOC,
							IP_HDR_LEN);
					CheckSum = ~CheckSum;
					*(TxFramePtr - IP_CSUM_LOC_BACK) =
								Xil_Htons(CheckSum);

					/*
					 * Echo reply status & checksum.
					 */
					Index = ICMP_ECHO_FIELD_LOC;
					while (Index < (ICMP_ECHO_FIELD_LOC +
							ICMP_ECHO_FIELD_LEN)) {
						*TxFramePtr++ = 0x0000;
						Index++;
					}

					/*
					 * Add data to buffer which was
					 * received from the packet.
					 */
					Index = ICMP_DATA_LOC;
					while (Index < (ICMP_DATA_LOC +
							ICMP_DATA_LEN)) {
						*TxFramePtr++ =
								(*(RxFramePtr + Index));
						Index++;
					}

					/*
					 * Generate checksum for the data and
					 * add it in the appropriate field.
					 */
					CheckSum = CheckSumCalculation(
							(u16 *)TxFrame,
							ICMP_DATA_START_LOC,
							ICMP_DATA_FIELD_LEN);
					CheckSum = ~CheckSum;
					*(TxFramePtr - ICMP_DATA_CSUM_LOC_BACK)
								= Xil_Htons(CheckSum);

					/*
					 * Transmit the frame.
					 */
#if 1
					 printf("\n%x. ICMP Tx Packet \n", NumOfPingReplies);
					 for(int i = 0; i < ICMP_PACKET_SIZE; i++)
					 	printf("%x ", TxFrame[i]);
					 printf("\n End of ICMP Tx Packet");
#endif					 
					XEmacLite_Send(InstancePtr,
							(u8 *)&TxFrame,
							ICMP_PACKET_SIZE);

					/*
					 * Increment the number of
					 * Ping replies sent.
					 */
					NumOfPingReplies++;
//					printf("NumOfPingReplies: %x", NumOfPingReplies);

				}
				else
					printf("\n Data check sum error");
			}
			else
				printf("\n IP Check sum error");
		}
	}
}

/*****************************************************************************/
/**
*
* This function calculates the checksum and returns a 16 bit result.
*
* @param 	RxFramePtr is a 16 bit pointer for the data to which checksum
* 		is to be calculated.
* @param	StartLoc is the starting location of the data from which the
*		checksum has to be calculated.
* @param	Length is the number of halfwords(16 bits) to which checksum is
* 		to be calculated.
*
* @return	It returns a 16 bit checksum value.
*
* @note		This can also be used for calculating checksum. The ones
* 		complement of this return value will give the final checksum.
*
******************************************************************************/
static u16 CheckSumCalculation(u16 *RxFramePtr, int StartLoc, int Length)
{
	u32 Sum = 0;
	u16 CheckSum = 0;
	int Index;

	/*
	 * Add all the 16 bit data.
	 */
	Index = StartLoc;
	while (Index < (StartLoc + Length)) {
		Sum = Sum + Xil_Ntohs(*(RxFramePtr + Index));
		Index++;
	}

	/*
	 * Add upper 16 bits to lower 16 bits.
	 */
	CheckSum = Sum;
	Sum = Sum >> 16;
	CheckSum = Sum + CheckSum;
	return CheckSum;
}
