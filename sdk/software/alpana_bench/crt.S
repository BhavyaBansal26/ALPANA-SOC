/* ALPANA_SOC benchmark startup: stack, .bss clear, FPU on, trap vector */
  .section .text.init
  .globl _start
_start:
  .option push
  .option norelax
  la gp, __global_pointer$
  .option pop
  la sp, _stack_top
  csrci mstatus, 8       /* boot ROM leaves MIE=1 with a timer pending */
  csrw mie, zero
  la t0, trap_entry
  csrw mtvec, t0
#ifdef __riscv_flen
  li t0, 0x6000          /* mstatus.FS = dirty */
  csrs mstatus, t0
  csrwi fcsr, 0
#endif
  la t0, __bss_start     /* gdb 'load' does not zero .bss on the board */
  la t1, __bss_end
1: bgeu t0, t1, 2f
  sw zero, 0(t0)
  addi t0, t0, 4
  j 1b
2:
  call uart_init
  call main
  call halt
3: j 3b

/* Trap entry. mcycle is sampled by the 3rd instruction for latency tests. */
  .align 4
  .globl trap_entry
trap_entry:
  addi sp, sp, -80
  sw t0, 0(sp)
  csrr t0, mcycle
  sw t0, trap_cycle, t1   /* t1 clobbered: save it right after */
  sw t1, 4(sp)
  sw ra, 8(sp)
  sw t2, 12(sp)
  sw a0, 16(sp)
  sw a1, 20(sp)
  sw a2, 24(sp)
  sw a3, 28(sp)
  sw a4, 32(sp)
  sw a5, 36(sp)
  sw a6, 40(sp)
  sw a7, 44(sp)
  sw t3, 48(sp)
  sw t4, 52(sp)
  sw t5, 56(sp)
  sw t6, 60(sp)
  csrr a0, mcause
  csrr a1, mepc
  call c_trap
  csrw mepc, a0
  lw t0, 0(sp)
  lw t1, 4(sp)
  lw ra, 8(sp)
  lw t2, 12(sp)
  lw a0, 16(sp)
  lw a1, 20(sp)
  lw a2, 24(sp)
  lw a3, 28(sp)
  lw a4, 32(sp)
  lw a5, 36(sp)
  lw a6, 40(sp)
  lw a7, 44(sp)
  lw t3, 48(sp)
  lw t4, 52(sp)
  lw t5, 56(sp)
  lw t6, 60(sp)
  addi sp, sp, 80
  mret
