# ALPANA SDK: load and run a program on ALPANA_SOC through OpenOCD
# usage: riscv32-unknown-elf-gdb -q -x alpana_tools/run.gdb <program>.shakti|.elf
set confirm off
set pagination off
set remotetimeout 60
target extended-remote localhost:3333
monitor halt
load
set $pc = _start
continue
