# ALPANA_SOC benchmark + stress suite  (Shakti E-class 4-stage RV32IMAFC, Nexys 4 DDR, 128 MB DDR2)
# Standalone bare-metal (own crt.S / lib.c), independent of the SDK library.
#   make            -> board ELFs in build/   (CPU_HZ=80 MHz, UART 115200)
#   make SIM=1      -> Verilator-sim ELFs (no baud write, signature exit)
# Toolchain: any riscv32/riscv64-unknown-elf-gcc with rv32imafc multilib
CROSS   ?= riscv32-unknown-elf-
CPU_HZ  ?= 80000000
BAUD_DIV?= 43
DHRY_RUNS ?= 20000
CM_ITER   ?= 400
STRESS_ROUNDS ?= 10
MEMTEST_KB    ?= 1024
CC      = $(CROSS)gcc
EXT    := $(shell $(CROSS)gcc -march=rv32imafc_zicsr_zifencei -mabi=ilp32f -x c -c /dev/null -o /dev/null 2>/dev/null && echo _zicsr_zifencei)
ARCH    = -march=rv32imafc$(EXT) -mabi=ilp32f
CFLAGS  = $(ARCH) -O2 -ffp-contract=off -fno-builtin -fno-common -nostdlib -nostartfiles \
          -ffreestanding -mcmodel=medany -I. -DCPU_HZ=$(CPU_HZ)u -DBAUD_DIV=$(BAUD_DIV) $(if $(SIM),-DSIM)
LDFLAGS = -T link.ld -Wl,--no-warn-rwx-segments
OUT     = build$(if $(SIM),_sim)
COMMON  = crt.S lib.c

all: $(OUT)/perf.elf $(OUT)/stress.elf $(OUT)/dhry.elf $(OUT)/coremark.elf

$(OUT)/perf.elf: perf.c $(COMMON) lib.h
	@mkdir -p $(OUT); $(CC) $(CFLAGS) $(LDFLAGS) $(COMMON) perf.c -lgcc -o $@

$(OUT)/stress.elf: stress.c stress_kern.h stress_list.h golden.h $(COMMON)
	@mkdir -p $(OUT); $(CC) $(CFLAGS) -DSTRESS_ROUNDS=$(STRESS_ROUNDS) -DMEMTEST_BYTES=$(MEMTEST_KB)*1024 \
	  $(LDFLAGS) $(COMMON) stress.c -lgcc -o $@

$(OUT)/dhry.elf: dhry/*.c dhry/*.h $(COMMON)
	@mkdir -p $(OUT); $(CC) $(CFLAGS) -Idhry -DNUMBER_OF_RUNS=$(DHRY_RUNS) $(LDFLAGS) $(COMMON) \
	  dhry/dhry_main.c dhry/dhrystone.c -lgcc -o $@ 2>/dev/null

$(OUT)/coremark.elf: cm/*.c cm/*.h $(COMMON)
	@mkdir -p $(OUT); $(CC) $(CFLAGS) -Icm -DITERATIONS=$(CM_ITER) -DPERFORMANCE_RUN=1 \
	  '-DFLAGS_STR="-O2 -march=rv32imafc"' $(LDFLAGS) $(COMMON) cm/core_list_join.c cm/core_main.c \
	  cm/core_matrix.c cm/core_state.c cm/core_util.c cm/core_portme.c -lgcc -o $@

# regenerate the host golden checksums (only needed if stress_kern.h / stress_list.h change)
golden.h: gen_golden.c stress_kern.h stress_list.h
	gcc -O2 -ffp-contract=off -o gen_golden gen_golden.c -lm && ./gen_golden > golden.h

clean:
	rm -rf build build_sim gen_golden
.PHONY: all clean
