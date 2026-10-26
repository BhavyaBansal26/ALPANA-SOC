#Created by Sathya Narayanan
# Email id: sathya281@gmail.com

#Copyright (C) 2020  IIT Madras. All rights reserved.

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
DC ?=
PROGRAM ?=
#default target board
TARGET ?= artix7_35t
DEBUG ?=
MARCH ?= rv32imac
MABI ?= ilp32
XLEN ?= 32
FLAGS ?=

all: create_dir
	make linkedlist.riscv

linkedlist.riscv: create_dir 
	@riscv$(XLEN)-unknown-elf-gcc -march=$(MARCH) -mabi=$(MABI) -w $(DC) -mcmodel=medany -static -std=gnu99 -fno-builtin-printf -I$(bspboard) -I$(bspinc)  -c $(bsplib)/sys_brk.c -o ./output/sys_brk.o 
	@riscv$(XLEN)-unknown-elf-gcc -march=$(MARCH) -mabi=$(MABI) -fno-zero-initialized-in-bss -w $(DC) -mcmodel=medany -static -std=gnu99 -fno-builtin-printf $(flags) -I$(bspinc) -I$(bspboard) -c $(bsplib)/malloc_firstfit.c -o ./output/malloc_firstfit.o
	@riscv$(XLEN)-unknown-elf-gcc -march=$(MARCH) -mabi=$(MABI) -w $(DC) -mcmodel=medany -static -std=gnu99 -fno-builtin-printf -I$(bspboard) -I$(bspinc) -c ./linkedlist.c -o ./output/linkedlist.o
	@riscv$(XLEN)-unknown-elf-gcc -march=$(MARCH) -mabi=$(MABI) -T  $(bspboard)/link.ld ./output/sys_brk.o ./output/malloc_firstfit.o ./output/linkedlist.o $(GENLIB)/gen_lib/libshakti$(XLEN).a  -o ./output/$(PROGRAM).shakti -static -nostartfiles
	@riscv$(XLEN)-unknown-elf-objdump -D ./output/$(PROGRAM).shakti > ./output/$(PROGRAM).dump

create_dir:
	@mkdir -p ./output

