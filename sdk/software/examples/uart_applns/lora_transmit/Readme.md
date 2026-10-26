
## Setup the LORA module
 * AT ==> response +OK
 * AT+MODE = 0  ==> response +OK
 * AT+BAND = 865000000   ==> response +OK

## The work mechanism of the LORA transmit
Set the address to the LORA mode for transmit address to 1
 * AT+ADDRESS = 1  ==> response +OK

To transmit a data or a message to the specified address set for Lora module.<br>
For example:<br>
- AT+SEND=0,5,Hello<br>
- AT+SEND=[Address],[Payload_Length],[Data]<br>
    - [Address]0~65535, When the [Address] is 0, it will    send data to all address (From 0 to 65535.)<br>
    - [Payload_Length] Maximum 240bytes<br>
    - [Data] ASCII Format
