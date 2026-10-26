/************************** Constant Definitions *****************************/
/**
 * Register offsets for the Ethernet MAC.
 */
#define XEL_TXBUFF_OFFSET (0x00000000)			/**< Transmit Buffer */
#define XEL_MDIOADDR_OFFSET (XEL_TXBUFF_OFFSET + 0x07E4)/**< MDIO Address offset
							     register */
#define XEL_MDIOWR_OFFSET  (XEL_TXBUFF_OFFSET + 0x07E8)	/**< MDIO write data
							     register offset */
#define XEL_MDIORD_OFFSET (XEL_TXBUFF_OFFSET + 0x07EC)	/**< MDIO read data
							     register offset*/
#define XEL_MDIOCNTR_OFFSET (XEL_TXBUFF_OFFSET + 0x07F0)/**< MDIO Control
							     Register offset */
#define XEL_GIER_OFFSET   (XEL_TXBUFF_OFFSET + 0x07F8)	/**< Offset for the GIE
							     Register */
#define XEL_TSR_OFFSET	  (XEL_TXBUFF_OFFSET + 0x07FC)	/**< Tx status */
#define XEL_TPLR_OFFSET   (XEL_TXBUFF_OFFSET + 0x07F4)	/**< Tx packet length */

#define XEL_RXBUFF_OFFSET (0x00001000)			/**< Receive Buffer */
#define XEL_RSR_OFFSET	  (XEL_RXBUFF_OFFSET + 0x07FC)	/**< Rx status */
#define XEL_RPLR_OFFSET   (XEL_RXBUFF_OFFSET + 0x0C)	/**< Rx packet length */

#define XEL_MAC_HI_OFFSET (XEL_TXBUFF_OFFSET + 0x14)	/**< MAC address hi
							     offset */
#define XEL_MAC_LO_OFFSET (XEL_TXBUFF_OFFSET)		/**< MAC address lo
							     offset */

#define XEL_BUFFER_OFFSET (0x00000800)			/**< Next buffer's
							     offset  same for
							     both TX and RX */
/**
 * MDIO Address/Write Data/Read Data Register Bit Masks
 */
#define XEL_MDIO_ADDRESS_MASK		0x00003E0	/**< PHY Address mask */
#define XEL_MDIO_ADDRESS_SHIFT		0x5		/**< PHY Address shift*/
#define XEL_MDIO_OP_MASK		0x00000400	/**< PHY read access */

/**
 * MDIO Control Register Bit Masks
 */
#define XEL_MDIOCNTR_STATUS_MASK	0x00000001	/**< MDIO transfer in
							     Progress */
#define XEL_MDIOCNTR_ENABLE_MASK	0x00000008	/**<  MDIO Enable */

/**
 * Global Interrupt Enable Register (GIER) Bit Masks
 */
#define XEL_GIER_GIE_MASK		0x80000000	/**< Global Enable */

/**
 * Transmit Status Register (TSR) Bit Masks
 */
#define XEL_TSR_XMIT_BUSY_MASK		0x00000001	/**< Xmit complete */
#define XEL_TSR_PROGRAM_MASK		0x00000002	/**< Program the MAC
							     address */
#define XEL_TSR_XMIT_IE_MASK		0x00000008	/**< Xmit interrupt
							     enable bit */
#define XEL_TSR_LOOPBACK_MASK		0x00000010	/**< Loop back enable
							     bit */
#define XEL_TSR_XMIT_ACTIVE_MASK	0x80000000	/**< Buffer is active,
							     SW bit only. This
							     is not documented
							     in the HW spec */

/**
 * define for programming the MAC address into the EmacLite
 */
#define XEL_TSR_PROG_MAC_ADDR   (XEL_TSR_XMIT_BUSY_MASK | XEL_TSR_PROGRAM_MASK)

/**
 * Receive Status Register (RSR)
 */
#define XEL_RSR_RECV_DONE_MASK		0x00000001	/**< Recv complete */
#define XEL_RSR_RECV_IE_MASK		0x00000008	/**< Recv interrupt
							     enable bit */

/**
 * Transmit Packet Length Register (TPLR)
 */
#define XEL_TPLR_LENGTH_MASK_HI		0x0000FF00 /**< Transmit packet length
							  upper byte */
#define XEL_TPLR_LENGTH_MASK_LO		0x000000FF /**< Transmit packet length
							  lower byte */

/**
 * Receive Packet Length Register (RPLR)
 */
#define XEL_RPLR_LENGTH_MASK_HI		0x0000FF00 /**< Receive packet length
							  upper byte */
#define XEL_RPLR_LENGTH_MASK_LO		0x000000FF /**< Receive packet length
							  lower byte */

#define XEL_HEADER_SIZE			14  /**< Size of header in bytes */
#define XEL_MTU_SIZE			1500 /**< Max size of data in frame */
#define XEL_FCS_SIZE			4    /**< Size of CRC */

#define XEL_HEADER_OFFSET		12   /**< Offset to length field */
#define XEL_HEADER_SHIFT		16   /**< Right shift value to align
						  length */


#define XEL_MAX_FRAME_SIZE (XEL_HEADER_SIZE+XEL_MTU_SIZE+ XEL_FCS_SIZE)	/**< Max
						length of Rx frame  used if
						length/type field
						contains the type (> 1500) */

#define XEL_MAX_TX_FRAME_SIZE (XEL_HEADER_SIZE + XEL_MTU_SIZE)	/**< Max
						length of Tx frame */


#define XEL_MAC_ADDR_SIZE		6	/**< length of MAC address */


/*
 * General Ethernet Definitions
 */
#define XEL_ETHER_PROTO_TYPE_IP		0x0800  /**< IP Protocol */
#define XEL_ETHER_PROTO_TYPE_ARP	0x0806  /**< ARP Protocol */
#define XEL_ETHER_PROTO_TYPE_VLAN	0x8100  /**< VLAN Tagged */
#define XEL_ARP_PACKET_SIZE		28  	/**< Max ARP packet size */
#define XEL_HEADER_IP_LENGTH_OFFSET	16  	/**< IP Length Offset */
#define XEL_VLAN_TAG_SIZE		4  	/**< VLAN Tag Size */

/***************** Macros (Inline Functions) Definitions *********************/

#define XEmacLite_In32 Xil_In32
#define XEmacLite_Out32 Xil_Out32

