#Created by Sathya Narayanan N & Abhinav Ramnath
# Email id: sathya281@gmail.com & abhinavramnath13@gmail.com

#   Copyright (C) 2019  IIT Madras. All rights reserved.

#This program is free software: you can redistribute it and/or modify
#it under the terms of the GNU General Public License as published by
#the Free Software Foundation, either version 3 of the License, or
#(at your option) any later version.

#This program is distributed in the hope that it will be useful,
#but WITHOUT ANY WARRANTY; without even the implied warranty of
#MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#   GNU General Public License for more details.

#    You should have received a copy of the GNU General Public License
#    along with this program.  If not, see <https://www.gnu.org/licenses/>.

SHELL := /bin/bash # Use bash syntax

export XLEN
export MABI
export MARCH
export FLAGS
export DC
export filepath

# appln and target board
PROGRAM ?=all
TARGET ?=artix7_35t
DEBUG ?=
UPLOAD ?=
FLASH ?=
XLEN ?=32
filepath ?= 
D ?= 
OPENOCD ?= $(shell which openocd)

GENLIB:=$(shell pwd)/../../software/projects/
# Invoke all possible combinations of applns for different targets
# There is a provision to make in the exact folder where code is written
# We are using that provision to invole appropriate makefile scripts from top folder
all: check library do upload flash finish
	@echo "TARGET is" $(TARGET)
check:
ifeq ($(DEBUG),DEBUG)
	@$(eval D = -g)
endif
ifeq ($(TARGET),artix7_35t)
	@$(eval XLEN = 32)
	@$(eval FLAGS= -D ARTIX7_35T)
	@$(eval MARCH=rv32imac)
	@$(eval MABI=ilp32)
	@$(eval INTERFACE=$(bspboard)/ftdi)
	@$(eval DC =$(FLAGS) $(D) )
else
ifeq ($(TARGET),artix7_100t)
	@$(eval XLEN = 64)
	@$(eval FLAGS= -D ARTIX7_100T)
	@$(eval MARCH=rv64imafdc)
	@$(eval MABI=lp64d)
	@$(eval INTERFACE=$(bspboard)/ftdi)
	@$(eval DC =$(FLAGS) $(D) )
else
ifeq ($(TARGET),moushik)
	@echo $(filepath)
	@$(eval XLEN = 32)
	@$(eval FLAGS= -D AARDONYX)
	@$(eval MARCH=rv32imac)
	@$(eval MABI=ilp32)
	@$(eval UPLOADER=$(UPLOADDIR)/micron)
	@$(eval FLASHSPEC=$(bspdri)/qspi/qspi_micron.c)
	@$(eval INTERFACE=$(bspboard)/jlink)
	@$(eval DC =$(FLAGS) $(D) )
else
ifeq ($(TARGET),pinaka)     # e-class 35t rv32imac
	@echo $(filepath)
	@$(eval XLEN = 32)
	@$(eval WIDTH = 4)
	@$(eval FLAGS= -D pinaka )
	@$(eval MARCH=rv32imac)
	@$(eval MABI=ilp32)
	@$(eval INTERFACE=$(bspboard)/ftdi)
	@$(eval DC=$(FLAGS) $(D) )
	@$(eval UPLOADER = $(UPLOADDIR)/spansion)
else
ifeq ($(TARGET),parashu)  # e-class 100t rv32imac
	@echo $(filepath)
	@$(eval XLEN = 32)
	@$(eval WIDTH = 4)
	@$(eval FLAGS = -D parashu )
	@$(eval MARCH=rv32imac)
	@$(eval MABI=ilp32)
	@$(eval UPLOADER = $(UPLOADDIR)/spansion)
	@$(eval INTERFACE=$(bspboard)/ftdi)
	@$(eval DC=$(FLAGS) $(D))
else
ifeq ($(TARGET),vajra)  # c-class 100t rv64imacsu
	@echo $(filepath)
	@$(eval XLEN = 64)
	@$(eval WIDTH = 8 )
	@$(eval FLAGS = -D vajra )
	@$(eval MARCH=rv64imac)
	@$(eval MABI=lp64)
	@$(eval UPLOADER= $(UPLOADDIR)/spansion)
	@$(eval INTERFACE = $(bspboard)/ftdi)
	@$(eval DC=$(FLAGS) $(D))
else
	@echo -e "$(TARGET) not supported"
endif
endif
endif
endif
endif
endif

library: 
ifeq ($(CLEAR),)
	make compile build
endif
compile:
	@mkdir -p ./gen_lib
	@echo "Compiling $(bspdri)"
	@echo $(bspboard)
	@riscv$(XLEN)-unknown-elf-gcc -w $(DC) -mcmodel=medany -static -std=gnu99 -fno-builtin-printf  $(FLAGS) -I$(bspinc) -c $(bspdri)/qspi/qspi_micron.c -o ./gen_lib/qspi_micron.o -march=$(MARCH) -mabi=$(MABI) -lm -lgcc
	@riscv$(XLEN)-unknown-elf-gcc -w $(DC) -mcmodel=medany -static -std=gnu99 -fno-builtin-printf $(FLAGS) -I$(bspinc) -I$(bspboard) -c $(bspdri)/pwm/pwm_driver.c -o ./gen_lib/pwm_driver.o -march=$(MARCH) -mabi=$(MABI) -lm -lgcc
	@riscv$(XLEN)-unknown-elf-gcc -w $(DC) -mcmodel=medany -static -std=gnu99 -fno-builtin-printf  $(FLAGS) -I$(bspinc) -I$(bspboard) -c $(bspdri)/uart/uart.c -o ./gen_lib/uart.o -march=$(MARCH)  -mabi=$(MABI) -lm -lgcc
	@riscv$(XLEN)-unknown-elf-gcc -w $(DC) -mcmodel=medany -static -std=gnu99 -fno-builtin-printf  $(FLAGS) -I$(bspinc) -I$(bspboard) -c $(bspdri)/i2c/i2c_driver.c -o ./gen_lib/i2c_driver.o -march=$(MARCH)	-mabi=$(MABI) -lm -lgcc
	@riscv$(XLEN)-unknown-elf-gcc -march=$(MARCH) -mabi=$(MABI)  -mcmodel=medany -static -std=gnu99 -fno-common -fno-builtin-printf $(FLAGS) -D__ASSEMBLY__=1 -I$(bspinc) -c $(bspcore)/start.S -o ./gen_lib/start.o
	@riscv$(XLEN)-unknown-elf-gcc -march=$(MARCH) -mabi=$(MABI)  -mcmodel=medany -static -std=gnu99 -fno-common -fno-builtin-printf $(FLAGS) -D__ASSEMBLY__=1 -I$(bspinc) -c $(bspcore)/trap.S -o ./gen_lib/trap.o
	@riscv$(XLEN)-unknown-elf-gcc -march=$(MARCH)  -mabi=$(MABI) -mcmodel=medany -static -std=gnu99 -fno-common -fno-builtin-printf $(FLAGS) -I$(bspinc) -I$(bspboard) -c $(bsplib)/printf.c -o ./gen_lib/printf.shakti
	@riscv$(XLEN)-unknown-elf-gcc -w $(DC) -mcmodel=medany -static -std=gnu99 -fno-builtin-printf  $(FLAGS)  -I$(bspinc)  -I$(bspboard) -c $(bsplib)/log.c -o ./gen_lib/log.o -march=$(MARCH) -mabi=$(MABI) -lm -lgcc
	@riscv$(XLEN)-unknown-elf-gcc -w $(DC) -mcmodel=medany -static -std=gnu99 -fno-builtin-printf   $(FLAGS) -I$(bspinc) -c $(bsplib)/util.c -o ./gen_lib/util.o -march=$(MARCH) -mabi=$(MABI)  -lm -lgcc
	@riscv$(XLEN)-unknown-elf-gcc -w $(DC) -mcmodel=medany -static -std=gnu99 -fno-builtin-printf   $(FLAGS) -I$(bspinc) -c $(bspcore)/traps.c -o ./gen_lib/traps.o -march=$(MARCH) -mabi=$(MABI)  -lm -lgcc
	@riscv$(XLEN)-unknown-elf-gcc -w $(DC) -mcmodel=medany -static -std=gnu99 -fno-builtin-printf $(FLAGS) -I$(bspinc) -I$(bspboard) -I$(bspdri)/plic -c $(bspcore)/init.c -o ./gen_lib/init.o -march=$(MARCH) -mabi=$(MABI)  -lm -lgcc
	@riscv$(XLEN)-unknown-elf-gcc -w $(DC) -mcmodel=medany -static -std=gnu99 -fno-builtin-printf   $(FLAGS) -I$(bspinc) -I$(bspboard) -c $(bspdri)/plic/plic_driver.c -o ./gen_lib/plic_driver.o         -march=$(MARCH) -mabi=$(MABI)  -lm -lgcc
	@riscv$(XLEN)-unknown-elf-gcc -w $(DC) -mcmodel=medany -static -std=gnu99 -fno-builtin-printf   $(FLAGS) -I$(bspinc) -I$(bspboard) -c $(bspdri)/clint/clint_driver.c -o ./gen_lib/clint_driver.o         -march=$(MARCH) -mabi=$(MABI)  -lm -lgcc
build:
	@riscv$(XLEN)-unknown-elf-ar rcs ./gen_lib/libshakti$(XLEN).a ./gen_lib/start.o ./gen_lib/trap.o ./gen_lib/util.o ./gen_lib/traps.o ./gen_lib/log.o ./gen_lib/printf.shakti ./gen_lib/qspi_micron.o ./gen_lib/uart.o ./gen_lib/i2c_driver.o ./gen_lib/clint_driver.o ./gen_lib/plic_driver.o ./gen_lib/init.o ./gen_lib/pwm_driver.o 
	
do:
ifeq ($(PROGRAM),all)
	cd ./intruder_detection && $(MAKE) all TARGET=$(TARGET)
	cd ./flash_gpio && $(MAKE) all TARGET=$(TARGET)
	cd ./irrigationsystem && $(MAKE) all TARGET=$(TARGET)
	cd ./weatherstation && $(MAKE) all TARGET=$(TARGET)
	cd ./weatherstation_bmp280 && $(MAKE) all TARGET=$(TARGET)
else 
ifeq ($(PROGRAM),lora_transmit)
filepath := ./lora/lora_transmit
else
ifeq ($(PROGRAM),lora_receive)
filepath := ./lora/lora_receive
else
ifeq ($(PROGRAM),intruder_detection)
filepath := ./intruder_detection
else
ifeq ($(PROGRAM),esp8266_bmp280)
filepath := ./esp8266_bmp280
else
ifeq ($(PROGRAM),irrigationsystem)
filepath := ./irrigationsystem
else
ifeq ($(PROGRAM),weatherstation_bmp280)
filepath := ./weatherstation_bmp280
else
ifeq ($(PROGRAM),weatherstation)
filepath := ./weatherstation
else
ifeq ($(PROGRAM),fingerprint)
filepath := ./fingerprint
else
ifeq ($(PROGRAM),flash_gpio)
filepath := ./flash_gpio
else
ifeq ($(PROGRAM),uart-cam)
filepath := ./uart-cam/ucam
else
endif
endif
endif
endif
endif
endif
endif
endif
endif
endif
endif

erase: check library 
	@rm -r output
	@mkdir output
	@riscv$(XLEN)-unknown-elf-gcc -march=$(MARCH) -mabi=$(MABI) -w $(DC) -mcmodel=medany -static -std=gnu99 -fno-builtin-printf -I$(bspboard) -I$(bspinc)  -c ../../bsp/utils/uploader/spansion/erase.c -o ./gen_lib/erase.o 
	@riscv$(XLEN)-unknown-elf-gcc -march=$(MARCH) -mabi=$(MABI) -w $(DC) -mcmodel=medany -static -std=gnu99 -fno-builtin-printf -I$(bspinc) -c $(bspdri)/spi/spi_spansion.c -o gen_lib/flash_driver.o  
	@riscv$(XLEN)-unknown-elf-gcc -march=$(MARCH) -mabi=$(MABI) -T  $(bspboard)/link.ld $(GENLIB)/gen_lib/libshakti$(XLEN).a gen_lib/flash_driver.o ./gen_lib/erase.o -o ./gen_lib/flasherase.shakti -static -nostartfiles 
	
upload: library do finish do1 path
path:
FILEPATH:='"$(GENLIB)$(filepath)"'

do1:
ifeq ($(UPLOAD),UPLOAD)
	@elf2hex 4 32768 $(filepath)/output/$(PROGRAM).shakti 2147483648 > $(filepath)/output/code.mem
	@gcc -g -w -D FILEPATH='"$(FILEPATH)"' $(UPLOADDIR)/elf_to_header.c -o $(filepath)/output/elf_to_header
	@$(filepath)/output/elf_to_header
	@riscv$(XLEN)-unknown-elf-gcc -w $(DC) -mcmodel=medany -static -std=gnu99 -fno-builtin-printf $(FLAGS) -I$(bspboard) -I$(bspinc) -I$(filepath)/output -c $(UPLOADER)/deploy.c -o $(filepath)/output/deploy.o -march=$(MARCH) -mabi=$(MABI) -lm -lgcc
	@riscv$(XLEN)-unknown-elf-gcc -w $(DC) -mcmodel=medany -static -std=gnu99 -fno-builtin-printf -I$(bspinc) -c $(FLASHSPEC) -o $(filepath)/output/flash_driver.o -march=$(MARCH) -mabi=$(MABI) -lm -lgcc
	@riscv$(XLEN)-unknown-elf-gcc -T $(bspboard)/link.ld $(filepath)/output/deploy.o $(filepath)/output/flash_driver.o $(GENLIB)/gen_lib/libshakti$(XLEN).a -o $(filepath)/output/deploy.shakti -static -nostartfiles -march=$(MARCH) -mabi=$(MABI) -lm -lgcc
	@riscv$(XLEN)-unknown-elf-objdump -D $(filepath)/output/deploy.shakti > $(filepath)/output/deploy.dump
	@echo -e "Upload elf to Flash"
#	@$(UPLOADDIR)/burnFlash.py ./genlib/deploy.shakti
#	@sudo $(OPENOCD) -f $(INTERFACE).cfg -c "reset init" -c "load_image $(filepath)/output/deploy.shakti; resume 0x80000000;shutdown"
	@riscv$(XLEN)-unknown-elf-objcopy -O ihex $(filepath)/output/deploy.shakti $(filepath)/output/deploy.hex
	@sudo $(OPENOCD) -f $(INTERFACE).cfg -c "reset init" -c "load_image $(filepath)/output/deploy.hex ;resume 0x80000000;shutdown"
	@rm -rf ./gen_lib/*.o
endif


flash:
ifeq ($(FLASH),FLASH) 	
	@flashrom -p ft2232_spi:type=arm-usb-tiny-h > ./$(filepath)/output/flashoutput.txt
	make moushik
endif
moushik : flashsize flashcheck objectsize swap filesize fill layoutsize layout flashrom clean
flashsize:
	@echo "Calculating flash size"
	@$(eval FLASHROM = $(shell egrep -o -e '[0-9]+\ kB' ./$(filepath)/output/flashoutput.txt | egrep -o '[0-9]+') )
	@$(eval FLASHSIZE = $(shell echo  $(FLASHROM)*1024  | bc ) )
	@echo "Checking for flash"
flashcheck:
	@echo "Size of flash attached is" $(FLASHSIZE)
#	Converts .shakti file to binary file	
	@riscv32-unknown-elf-objcopy -O binary ./$(filepath)/output/$(PROGRAM).shakti ./$(filepath)/output/output.bin      

objectsize:
#Find size of output.bin and append it to the binary file
	$(eval FILE = $(shell stat -c %s ./$(filepath)/output/output.bin))
swap:
	@echo "The size of the bin file is : " $(FILE) 
#	Swaps endianness from little-endian to big-endian as the flash programmer swaps endianness during write.
	@hexdump -v -e '1/4 "%08x"' -e '"\n"' ./$(filepath)/output/output.bin | xxd -r -p > ./$(filepath)/output/bigoutput.bin	
	@printf "%08x\n" $(FILE) > ./$(filepath)/output/size.txt
#	Appending file size to bin file	
	@xxd -r -p ./$(filepath)/output/size.txt ./$(filepath)/output/newoutput.bin
	@cat ./$(filepath)/output/bigoutput.bin >> ./$(filepath)/output/newoutput.bin
filesize:
	@$(eval FILESIZE=$(shell stat -c%s ./$(filepath)/output/newoutput.bin) )
fill:
	@$(eval SPACE = $(shell echo  $(FLASHSIZE)-$(FILESIZE)  | bc ) )
	@echo "Expanding bin to flash size. This might take a few seconds.."
#the file is expanded to 32kb and appended with 0's where required.
	@dd if=/dev/zero of=./$(filepath)/output/newoutput.bin bs=1 count=$(SPACE) conv=notrunc oflag=append
layoutsize:
	@printf "%x\n" $(FLASHSIZE) > ./$(filepath)/output/size1.txt	
layout:
	@$(eval FILESIZE= $(shell cat ./$(filepath)/output/size.txt) )
	@echo $(FILESIZE)
	@echo "0000:$(FILESIZE) flash" > ./$(filepath)/output/layout.txt
flashrom:
	@flashrom -p ft2232_spi:type=arm-usb-tiny-h -l ./$(filepath)/output/layout.txt -i flash -w ./$(filepath)/output/newoutput.bin
clean:
	@rm -rf ./$(filepath)/output/*.txt
	@rm -rf ./$(filepath)/output/*.bin
	@find -type d -name output -a -prune -exec rm -rf {} \;
	

finish:
ifeq ($(PROGRAM),all)
	@echo -e "make all over"
else
ifeq ($(CLEAR),)
	cd $(filepath) && $(MAKE) $(PROGRAM).riscv TARGET=$(TARGET) DEBUG=$(DEBUG)
else
ifeq ($(PROGRAM),)
	cd $(filepath)/../ && $(MAKE) clean CLEAR=$(CLEAR)
else
	cd $(filepath)/../ && $(MAKE) clean PROGRAM=$(PROGRAM) CLEAR=$(CLEAR)
endif
endif
endif
