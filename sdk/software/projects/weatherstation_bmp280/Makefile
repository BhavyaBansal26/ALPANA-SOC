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

TARGET = artix7_35t
BSP:=../../../bsp
#bsp incl file path
bspinc:=$(BSP)/include
#bsp lib file path
bsplib:=$(BSP)/libs
#bsp board specific files path
bspboard:=$(BSP)/third_party/$(TARGET)/
bspdri:=$(BSP)/drivers
bsputils:=$(BSP)/utils
bspcore:=$(BSP)/core
DEBUG ?=
PROGRAM=weatherstation_bmp280
D ?= -g
XLEN ?= 32
WIDTH ?= 4
FLAGS ?= -D ARTIX7_35T
MARCH ?= rv32imac
MABI ?= ilp32
DC = $(FLAGS) $(D)


all: create_dir
	make $(PROGRAM).riscv

upload: all
	@elf2hex 4 16384 ./output/$(PROGRAM).shakti 2147483648 > ./output/code.mem
	@echo -e "Converting Codemem to array"
	@gcc -g -w  $(bsputils)/elf_to_header.c -o ./output/elf_to_header
	@./output/elf_to_header
	@riscv$(XLEN)-unknown-elf-gcc -w $(DC) -mcmodel=medany -static -std=gnu99 -fno-builtin-printf $(flags) -I$(bspinc) -I./output -c $(bsputils)/deploy.c -o ./output/deploy.o -march=$(MARCH) -lm -lgcc
	@riscv$(XLEN)-unknown-elf-gcc -w $(DC) -mcmodel=medany -static -std=gnu99 -fno-builtin-printf  -I$(bspinc) -c $(bspboard)/uart.c -o ./output/uart.o -march=$(MARCH) -lm -lgcc
	@riscv$(XLEN)-unknown-elf-gcc -w $(DC) -mcmodel=medany -static -std=gnu99 -fno-builtin-printf  -I$(bspinc) -c $(bspdri)/spi/spi_driver.c -o ./output/spi_driver.o -march=$(MARCH) -lm -lgcc
	@riscv$(XLEN)-unknown-elf-gcc -T $(bspboard)/link.ld ./output/deploy.o ./output/syscalls.shakti ./output/uart.o ./output/crt.o ./output/spi_driver.o -o ./output/deploy.shakti -static -nostartfiles
	@echo -e "Upload elf to Flash"
	@$(bsputils)/burnFlash.py ./output/deploy.shakti


$(PROGRAM).riscv: create_dir compile build
	@riscv$(XLEN)-unknown-elf-gcc -w $(DC) -mcmodel=medany -static -std=gnu99 -fno-builtin-printf $(flags) -I$(bspinc)  -I$(bspboard) -c ./$(PROGRAM).c -o ./output/$(PROGRAM).o -march=$(MARCH) -mabi=$(MABI) -lm -lgcc
	@riscv$(XLEN)-unknown-elf-gcc -w $(DC) -mcmodel=medany -static -std=gnu99 -fno-builtin-printf  -I$(bspinc) -I$(bspboard) -c ./bmp280.c -o ./output/lm75.o -march=$(MARCH) -mabi=$(MABI) -lm -lgcc		
	@riscv$(XLEN)-unknown-elf-gcc -march=$(MARCH) -mabi=$(MABI) -T  $(bspboard)/link.ld ./output/lm75.o ./gen_lib/libshakti$(XLEN).a ./output/$(PROGRAM).o -o ./output/$(PROGRAM).shakti -static -nostartfiles -lm -lgcc
	@riscv$(XLEN)-unknown-elf-objdump -D ./output/$(PROGRAM).shakti > ./output/$(PROGRAM).dump


create_dir:
	@mkdir -p ./output


crt.o: create_dir
	@riscv$(XLEN)-unknown-elf-gcc -march=$(MARCH) -mcmodel=medany -static -std=gnu99 -fno-common -fno-builtin-printf -D__ASSEMBLY__=1 -c $(bspinc)/crt.S -o ./output/crt.o

compile:
	@mkdir -p ./gen_lib
	@riscv$(XLEN)-unknown-elf-gcc -march=$(MARCH) -mabi=$(MABI) -w $(DC) -mcmodel=medany -static -std=gnu99 -fno-builtin-printf  -I$(bspinc) -c $(bsplib)/util.c -o ./gen_lib/util.o -lm -lgcc
	@riscv$(XLEN)-unknown-elf-gcc  -march=$(MARCH) -mabi=$(MABI) -w $(DC) -mcmodel=medany -static -std=gnu99 -fno-builtin-printf -I$(bspinc) -c $(bspcore)/traps.c -o ./gen_lib/traps.o  -lm -lgcc
	@riscv$(XLEN)-unknown-elf-gcc  -march=$(MARCH) -mabi=$(MABI) -w $(DC) -mcmodel=medany -static -std=gnu99 -fno-builtin-printf -I$(bspinc) -I$(bspboard) -c $(bspdri)/pwm/pwm_driver.c -o ./gen_lib/pwm_driver.o -lm -lgcc
	@riscv$(XLEN)-unknown-elf-gcc  -march=$(MARCH) -mabi=$(MABI) -w $(DC) -mcmodel=medany -static -std=gnu99 -fno-builtin-printf -I$(bspinc) -I$(bspboard) -c $(bspdri)/uart/uart.c -o ./gen_lib/uart.o -lm -lgcc
	@riscv$(XLEN)-unknown-elf-gcc  -march=$(MARCH) -mabi=$(MABI) -w $(DC) -mcmodel=medany -static -std=gnu99 -fno-builtin-printf  -I$(bspinc) -I$(bspboard) -c $(bspdri)/i2c/i2c_driver.c -o ./gen_lib/i2c_driver.o  -lm -lgcc
	@riscv$(XLEN)-unknown-elf-gcc -march=$(MARCH) -mabi=$(MABI)  -mcmodel=medany -static -std=gnu99 -fno-common -fno-builtin-printf  -D__ASSEMBLY__=1 -I$(bspinc) -c $(bspcore)/start.S -o ./gen_lib/start.o
	@riscv$(XLEN)-unknown-elf-gcc -march=$(MARCH) -mabi=$(MABI)  -mcmodel=medany -static -std=gnu99 -fno-common -fno-builtin-printf  -D__ASSEMBLY__=1 -I$(bspinc) -c $(bspcore)/trap.S -o ./gen_lib/trap.o
	@riscv$(XLEN)-unknown-elf-gcc -march=$(MARCH)  -mabi=$(MABI) -mcmodel=medany -static -std=gnu99 -fno-common -fno-builtin-printf  -I$(bspinc) -I$(bspboard) -c $(bsplib)/printf.c -o ./gen_lib/printf.shakti
	@riscv$(XLEN)-unknown-elf-gcc  -march=$(MARCH) -mabi=$(MABI) -w $(DC) -mcmodel=medany -static -std=gnu99 -fno-builtin-printf  -I$(bspinc)  -I$(bspboard) -c $(bsplib)/log.c -o ./gen_lib/log.o -lm -lgcc
	@riscv$(XLEN)-unknown-elf-gcc  -march=$(MARCH) -mabi=$(MABI) -w $(DC) -mcmodel=medany -static -std=gnu99 -fno-builtin-printf -I$(bspinc) -I$(bspboard) -I$(bspdri)/plic -c $(bspcore)/init.c -o ./gen_lib/init.o  -lm -lgcc
	@riscv$(XLEN)-unknown-elf-gcc  -march=$(MARCH) -mabi=$(MABI) -w $(DC) -mcmodel=medany -static -std=gnu99 -fno-builtin-printf -I$(bspinc) -I$(bspboard) -c $(bspdri)/plic/plic_driver.c -o ./gen_lib/plic_driver.o          -lm -lgcc
	@riscv$(XLEN)-unknown-elf-gcc  -march=$(MARCH) -mabi=$(MABI) -w $(DC) -mcmodel=medany -static -std=gnu99 -fno-builtin-printf -I$(bspinc) -I$(bspboard) -c $(bspdri)/clint/clint_driver.c -o ./gen_lib/clint_driver.o        -lm -lgcc

build:
	@riscv$(XLEN)-unknown-elf-ar rcs ./gen_lib/libshakti$(XLEN).a ./gen_lib/start.o ./gen_lib/trap.o ./gen_lib/util.o ./gen_lib/traps.o ./gen_lib/log.o ./gen_lib/printf.shakti ./gen_lib/uart.o ./gen_lib/i2c_driver.o ./gen_lib/clint_driver.o ./gen_lib/plic_driver.o ./gen_lib/init.o ./gen_lib/pwm_driver.o

