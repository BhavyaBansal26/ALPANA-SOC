#Created by Sathya Narayanan N
# Email id: sathya281@gmail.com

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

export bsplib
export bspcore
export bsputil
export bspboard
export bspinc
export bspdri
export UPLOADDIR
export UPLOADER
export OCDPATH
export INTERFACE
export FLASHSPEC
export GENLIB

# Default PROGRAM and TARGET
PROGRAM ?= 
TARGET ?= alpana
DEBUG ?= DEBUG
CLEAR ?=
UPLOAD ?= UPLOAD
FLASH ?= FLASH
MARCH ?=
MABI  ?=
CLEAR ?=

#############################################################
# Prints help message
#############################################################
.PHONY: help
help:
	@echo " Shakti Software Development Kit "
	@echo " Makefile features:"
	@echo ""
	@echo " list_targets"
	@echo " lists the boards supported"
	@echo ""
	@echo " list_applns"
	@echo " lists the example applns availabel to user"
	@echo ""
	@echo " software [PROGRAM=$(PROGRAM)] [TARGET=$(TARGET)]"
	@echo " Builds the requested PROGRAM for the given TARGET"
	@echo ""
	@echo " debug [PROGRAM=$(PROGRAM)] [TARGET=$(TARGET)]"
	@echo " Builds the requested PROGRAM with debug options"
	@echo ""
	@echo " upload [PROGRAM=$(PROGRAM)] [TARGET=$(TARGET)]"
	@echo " Uploads the requested PROGRAM to the Flash memory in the board"
	@echo ""
	@echo " flash [PROGRAM=$(PROGRAM)] [TARGET=$(TARGET)]"
	@echo " Flashes the requested PROGRAM to the Flash memory in the board"
	@echo ""
	@echo " clean"
	@echo " Cleans compiled objects in every example applns."
	@echo ""
	@echo " clean [PROGRAM=$(PROGRAM)]"
	@echo " Cleans compiled objects of that particular appln."



#BOARD_DIR holds the list of boards in a third_party path
BOARD_DIR := $(shell ls ./bsp/third_party)


#Below variables hold the list of C files in there path
#Each C file corresponds to a appln. Each appln has a label in makefile
#that corresponds to each C file.

GPIO_APPS := $(shell cd ./software/examples/gpio_applns/ && ls -d * | grep -v Makefile )
UART_APPS := $(shell cd ./software/examples/uart_applns/ && ls -d * | grep -v Makefile )
I2C_APPS := $(shell cd software/examples/i2c_applns/ && ls -d * | grep -v Makefile )
PWM_APPS := $(shell cd software/examples/pwm_applns/ && ls -d * | grep -v Makefile )
SPI_APPS := $(shell cd software/examples/spi_applns/ && ls -d * | grep -v Makefile )
PLIC_APPS := $(shell cd software/examples/plic_applns/ && ls -d * | grep -v Makefile )
APP_DIR := $(GPIO_APPS) $(UART_APPS) $(I2C_APPS) $(SPI_APPS) $(PWM_APPS) $(PLIC_APPS)


#bsp board specific files path
BSP:=$(shell pwd)/bsp
bspboard:=$(BSP)/third_party/$(TARGET)/
bspdri:=$(BSP)/drivers
bsputil:=$(BSP)/utils
#bsp incl file path
bspinc:=$(BSP)/include
#bsp lib file path
bsplib:=$(BSP)/libs
#bsp board specific files path
bspboard:=$(BSP)/third_party/$(TARGET)
#bsp core file path
bspcore:=$(BSP)/core
UPLOADDIR :=$(bsputil)/uploader
UPLOADER :=$(UPLOADDIR)/spansion
FLASHSPEC :=$(bspdri)/spi/spi_spansion.c
INTERFACE :=$(bspboard)/ftdi
OCDPATH:=$(shell pwd)/shakti-tools/bin
GENLIB:=$(shell pwd)/software/examples/

#List the boards that are supported by Shakti Sdk
.PHONY: all
all:
	cd  ./software/examples/  && $(MAKE) all TARGET=$(TARGET)

.PHONY: target
list_targets:
	@echo $(sort $(BOARD_DIR))

#List the example applns running on Shakti
.PHONY: appln
list_applns:
	@echo $(sort $(APP_DIR))

#Software commands
.PHONY: software
software:
	@echo "Build $(PROGRAM) on $(TARGET) board"
	cd ./software/examples && $(MAKE) PROGRAM=$(PROGRAM) TARGET=$(TARGET)

#Project commands
.PHONY: project
project:
	@echo "Build $(PROGRAM) on $(TARGET) board"
	cd ./software/projects && $(MAKE) PROGRAM=$(PROGRAM) TARGET=$(TARGET)

.PHONY: upload
upload:
	@echo Build and upload $(PROGRAM) on $(TARGET) board
	cd ./software/examples && $(MAKE) UPLOAD=$(UPLOAD) PROGRAM=$(PROGRAM) TARGET=$(TARGET)

.PHONY: upload_project
upload_project:
	@echo Build and upload $(PROGRAM) on $(TARGET) board
	cd ./software/projects && $(MAKE) UPLOAD=$(UPLOAD) PROGRAM=$(PROGRAM) TARGET=$(TARGET)


.PHONY: erase
erase:
	@echo Erase on-board flash $(PROGRAM) on Arty board
	cd ./software/examples && $(MAKE) erase TARGET=$(TARGET)

.PHONY: flash
flash:
	@echo Build and flash $(PROGRAM) on $(TARGET) board
	cd ./software/examples && $(MAKE) FLASH=$(FLASH) PROGRAM=$(PROGRAM) TARGET=$(TARGET)

.PHONY: debug
debug:
	@echo Build $(PROGRAM) on $(TARGET) board with $(DEBUG)
	cd ./software/examples/ && $(MAKE) PROGRAM=$(PROGRAM) TARGET=$(TARGET) DEBUG=$(DEBUG)

.PHONY: clean
clean:
#remove the gen_lib directory
	@if [ -d software/examples/gen_lib/ ] ; then rm -rf software/examples/gen_lib/; fi
ifeq ($(PROGRAM),)
	cd ./software/examples/clint_applns && $(MAKE) clean CLEAR=CLEAR
	cd ./software/examples/spi_applns && $(MAKE) clean CLEAR=CLEAR
	cd ./software/examples/uart_applns && $(MAKE) clean CLEAR=CLEAR
	cd ./software/examples/i2c_applns && $(MAKE) clean CLEAR=CLEAR
	cd ./software/examples/gpio_applns && $(MAKE) clean CLEAR=CLEAR
	cd ./software/examples/pwm_applns && $(MAKE) clean CLEAR=CLEAR
	cd ./software/examples/plic_applns && $(MAKE) clean CLEAR=CLEAR
	cd ./software/examples/malloc_test && $(MAKE) clean CLEAR=CLEAR
	cd ./software/examples/xadc_applns && $(MAKE) clean CLEAR=CLEAR
	cd ./software/examples/eth_test && $(MAKE) clean CLEAR=CLEAR
	cd ./software/projects && $(MAKE) clean CLEAR=CLEAR
else
	cd ./software/examples && $(MAKE) PROGRAM=$(PROGRAM) CLEAR=CLEAR
endif
