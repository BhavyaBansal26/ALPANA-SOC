# ALPANA SDK application Makefile (built via: make software PROGRAM=<name>)
SHELL := /bin/bash
DC ?=
PROGRAM ?=
TARGET ?= alpana
DEBUG ?=
MARCH ?= rv32imafc
MABI ?= ilp32f
XLEN ?= 32
FLAGS ?=
EXTRA_SRC ?= 
OPT ?= -O2

all: create_dir
	make $(PROGRAM).riscv

$(PROGRAM).riscv: create_dir
	@riscv$(XLEN)-unknown-elf-gcc -march=$(MARCH) -mabi=$(MABI) $(OPT) -Wall $(DC) -mcmodel=medany -static -std=gnu99 -fno-builtin-printf $(FLAGS) -I. -I$(bsplib) -I$(bspinc) -I$(bspdri)/plic -I$(bspboard) ./$(PROGRAM).c $(EXTRA_SRC) -c
	@mv *.o ./output/
	@riscv$(XLEN)-unknown-elf-gcc -march=$(MARCH) -mabi=$(MABI) -T $(bspboard)/link.ld ./output/*.o $(GENLIB)/gen_lib/libshakti$(XLEN).a -o ./output/$(PROGRAM).shakti -static -nostartfiles -lm -lgcc
	@riscv$(XLEN)-unknown-elf-objdump -D ./output/$(PROGRAM).shakti > ./output/$(PROGRAM).dump
	@riscv$(XLEN)-unknown-elf-size ./output/$(PROGRAM).shakti
	@echo "built ./output/$(PROGRAM).shakti  ->  load with: riscv32-unknown-elf-gdb -x $(abspath $(bspboard)/../../../alpana_tools/run.gdb) ./output/$(PROGRAM).shakti"

create_dir:
	@mkdir -p ./output
	@rm -f ./output/*.o
