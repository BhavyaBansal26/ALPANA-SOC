
## Setup the LORA module
 * AT ==> response +OK
 * AT+MODE = 0  ==> response +OK
 * AT+BAND = 865000000   ==> response +OK

## The work mechanism of the LORA receive
Set the address to the LORA mode for transmit address to 0
 * AT+ADDRESS = 0  ==> response +OK

To receive the message or data you just have to read the value from the LORA