# Uart-cam

### Sample project to read data from Shakti using uart 

Connections : Connect to UART2 rx,tx pins of board by using a [UART to USB converter](https://robu.in/product/ft232rl-usb-to-ttl-5v-3-3v-download-cable-to-serial-adapter-module-for-arduino/). Run this code separately to read data.

Sample command :

$ make

gcc -std=gnu99 camera.c

Run the program as sudo ./a.out [port]

$ sudo ./a.out /dev/ttyUSB2
