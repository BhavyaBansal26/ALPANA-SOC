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

# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <https://www.gnu.org/licenses/>

SHELL := /bin/bash # Use bash syntax
XLEN ?=64
DC=

#default target board
TARGET ?= artix7_35t
DEBUG ?=
PROGRAM=irrigationsystem

ifeq ($(TARGET),artix7_35t)
	XLEN=32
	flags= -D ARTIX7_35T
	MARCH=rv32imac
	MABI=ilp32
endif

ifeq ($(TARGET),artix7_100t)
	XLEN=64
	flags= -D ARTIX7_100T
endif

ifeq ($(DEBUG),DEBUG)
	DC=-g
endif

BSP:=../../../bsp
#bsp incl file path
bspinc:=$(BSP)/include
#bsp lib file path
bsplib:=$(BSP)/libs
#bsp board specific files path
bspboard:=$(BSP)/third_party/$(TARGET)/
bspdri:=$(BSP)/drivers
bsputils:=$(BSP)/utils


all: create_dir
	make $(PROGRAM).riscv

$(PROGRAM).riscv: create_dir
	@riscv$(XLEN)-unknown-elf-gcc -march=$(MARCH) -mabi=$(MABI) -w $(DC) -mcmodel=medany -static -std=gnu99 -fno-builtin-printf $(FLAGS)-I$(bsplib) -I$(bspinc) -I$(bspdri)/plic -I$(bspboard) -c ./$(PROGRAM).c -o ./output/$(PROGRAM).o -lm -lgcc
	@riscv$(XLEN)-unknown-elf-gcc -march=$(MARCH) -mabi=$(MABI) -w $(DC) -mcmodel=medany -static -std=gnu99 -fno-builtin-printf $(FLAGS)-I$(bsplib) -I$(bspinc) -I$(bspdri)/plic -I$(bspboard) -c ./lm75.c -o ./output/lm75.o -lm -lgcc
	@riscv$(XLEN)-unknown-elf-gcc -march=$(MARCH) -mabi=$(MABI) -T  $(bspboard)/link.ld ./output/lm75.o $(GENLIB)gen_lib/libshakti$(XLEN).a ./output/$(PROGRAM).o -o ./output/$(PROGRAM).shakti -static -nostartfiles -lm -lgcc
	@riscv$(XLEN)-unknown-elf-objdump -D ./output/$(PROGRAM).shakti > ./output/$(PROGRAM).dump



create_dir:
	@mkdir -p ./output
