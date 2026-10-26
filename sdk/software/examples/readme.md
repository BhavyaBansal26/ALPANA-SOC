The readme is a brief overview on gpio, i2c, uart example applications. Next Releases will includes applications of qspi, spi and pwm support.

### GEN_LIB
   * This consists of the common library files that are built and need to be linked with the applications.
   * This consists of the following library files
      - uart driver
      - qspi_micron driver
      - crt.S
      - syscalls
      - log
      - util
      - traps
      - init
      - plic driver
      - clic driver
   * Note: this folder is generated after issuing make

### FLASHROM
   - This is the open-source library that we use to program external flash
   - Libraries required to install flashrom
   - sudo apt-get install libpci-dev flashrom libftdi-dev libusb-dev
   - flashrom automatically detects the flash. a list of supported flashes are available here : (https://flashrom.org/Supported_hardware)
     
### GPIO_APPLICATIONS
   - This consists of applications developed using the gpio pins.

#### Contents ####
 
* btnled
     - This program controls the led using the button.
     -  src code:btnled.c
     -  Test components :led,button.
* keypad
    - The program is all about how to get the value of a key when pressed on the board.
    -  src code:keyboard.c
    -  Test components :digilent pmod Keypad.
* motor
     - This is the program that tests the functioning of the  stepper motor using the gpio pins.
     -  src code:motor.c
     -  Test components: stepper motor.
* rdgpio
     - This is the program that reads the inputs given by the gpio pins.
     -  src code:rdgpio.c
* tglgpio
     - This is the program that performs the toggling of led.
     -  src code:tglgpio.c
     -  Test components:led
* gyro_softi2c
     - This works with gyro sensor using gpio based i2c.
     -  src code: gyroi2c.c
     -  Test components:mpu6050

### I2C_APPLICATIONS
- This consists of applications developed using the i2c communication protocol.

#### Contents ####

* lm75
   - The program performs the Temperature sensing using the i2c communication protocol.
   - src code:lm75.c
   - sensor used:lm75x

### UART_APPLICATIONS 
  -  This consists of applications developed using the uart protocol.

#### Contents ####

* hello
  - The program is used to test the uart with the help of hello world program.
  -  src code:hello.c
* maze
  - A configurable rat in a maze game.
  -  src code:maze.c
