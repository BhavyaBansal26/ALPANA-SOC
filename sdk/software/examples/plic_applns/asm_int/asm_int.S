/***************************************************************************
* Project               	: shakti devt board
* Name of the file	        : asm_int.S
* Brief Description of file     : Source asm file for interrupt demonstration.
* Name of Author    	        : Sathya narayanan
* Email ID                      : sathya281@gmail.com

 Copyright (C) 2019  IIT Madras. All rights reserved.

 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <https://www.gnu.org/licenses/>.
***************************************************************************/
/**
 * @file asm_int.S
 * @brief Source file for interrupt demo using asm program.
 * @detail This file is used to demonstrate the interrupt handling feature using asm.
 */

#start in m mode, jump to S mode. access M mode code. Trap happens. handle trap in M mode. jump to mepc

#if __riscv_xlen == 64
#define LREG ld
#define SREG sd
#define REGBYTES 8
#else
#define LREG lw
#define SREG sw
#define REGBYTES 4
#endif

_start:	
	andi sp,t2,0
	andi t1,t2,0
	andi t2,t2,0
	andi t3,t2,0
	andi t4,t2,0
	andi t5,t2,0
	andi t6,t2,0
	andi t0,t2,0
	andi a0,t2,0
	andi a1,t2,0
	andi a2,t2,0
	andi a3,t2,0
	andi a4,t2,0
	andi a5,t2,0
	andi a6,t2,0
	andi a7,t2,0
	andi s1,t2,0
	andi s2,t2,0
	andi s3,t2,0
	andi s4,t2,0
	andi s5,t2,0
	andi s6,t2,0
	andi s7,t2,0
	andi s8,t2,0
	andi s9,t2,0
	andi s10,t2,0
	andi s11,t2,0
	andi ra,t2,0
	andi gp,t2,0
	andi tp,t2,0
	andi fp,t2,0
	li sp, 0x80010000
	la t0, trap_entry
	csrw mtvec, t0              #trying to access mtvec
	li t2, 0x00011300
uart_status_simple:	lb t1, 12(t2)
	andi t1,t1,0x2
	bnez t1, uart_status_simple
	andi t1,t1,0
	addi t1,t1,65
	sb t1, 4(t2)
	jal ra,init_interrupt	#initialize interrupt
loop:	j loop

init_interrupt:
    li      t0, 8
	csrrs   zero, mstatus, t0  # Enable Global (PLIC) interrupt
	li      t0, 0x800
	csrrs   zero, mie, t0  #enable local external interrupt bit
	csrr s8,mstatus
	andi t1, s8, 8
	bnez t1, putchar
returning:
	andi t5,t5,0
	andi t6,t6,0
	addi t5, t5, 96
	andi t4,t4,0
	addi t4,t4,2
begin:	li t3, 0x0C000000    #plic priority address
	add t3,t3, t6
	sw t4, 0(t3)
	addi t6, t6, 4
	bge  t5, t6, begin

	andi t4,t4,0
	addi t4,t4,0xff            # enable individual interrupt
	li t3, 0x0C002000    #plic base address
	sb t4, 0(t3)
	li t3, 0x0C002001    #plic base address
	sb t4, 0(t3)
	li t3, 0x0C002002    #plic base address
	sb t4, 0(t3)
	li t3, 0x0C002003    #plic base address
	sb t4, 0(t3)

	li t3, 0x0c010000 #set threshold priority
	li t4, 0x1
	sw t4, 0(t3)
	ret

.p2align 2
trap_handler:
	li s3, 0x0c010010
	csrr t0,mcause
	li t3, 0x80000000
	and t0,t0,t3
	beqz t0, exception_handler // it is a exception
	beq t0, t3, interrupt_handler
1:      ret

.p2align 2
exception_handler:
	csrr t0,mcause
	la t1, _data1  #base address
	lw t2, 0(t1)  # offset value
	addi t2,t2,4 #t1 holds base address for storing trap occurences
	sw t2,0(t1)  
	add t1, t1,t2
	sw t0,0(t1)  
	j 1b


.p2align 2
trap_entry:     # currently trap_entry has a nop. Later we will add actual instrns
addi sp, sp, -32*REGBYTES
 nop
#x0 is always 0
SREG x1, 1*REGBYTES(sp)
SREG x2, 2*REGBYTES(sp)
SREG x3, 3*REGBYTES(sp)
SREG x4, 4*REGBYTES(sp)
SREG x5, 5*REGBYTES(sp)
SREG x6, 6*REGBYTES(sp)
SREG x7, 7*REGBYTES(sp)
SREG x8, 8*REGBYTES(sp)
SREG x9, 9*REGBYTES(sp)
SREG x10, 10*REGBYTES(sp)
SREG x11, 11*REGBYTES(sp)
SREG x12, 12*REGBYTES(sp)
SREG x13, 13*REGBYTES(sp)
SREG x14, 14*REGBYTES(sp)
SREG x15, 15*REGBYTES(sp)
SREG x16, 16*REGBYTES(sp)
SREG x17, 17*REGBYTES(sp)
SREG x18, 18*REGBYTES(sp)
SREG x19, 19*REGBYTES(sp)
SREG x20, 20*REGBYTES(sp)
SREG x21, 21*REGBYTES(sp)
SREG x22, 22*REGBYTES(sp)
SREG x23, 23*REGBYTES(sp)
SREG x24, 24*REGBYTES(sp)
SREG x25, 25*REGBYTES(sp)
SREG x26, 26*REGBYTES(sp)
SREG x27, 27*REGBYTES(sp)
SREG x28, 28*REGBYTES(sp)
SREG x29, 29*REGBYTES(sp)
SREG x30, 30*REGBYTES(sp)
SREG x31, 31*REGBYTES(sp)

jal trap_handler

LREG x1, 1*REGBYTES(sp)
LREG x2, 2*REGBYTES(sp)
LREG x3, 3*REGBYTES(sp)
LREG x4, 4*REGBYTES(sp)
LREG x5, 5*REGBYTES(sp)
LREG x6, 6*REGBYTES(sp)
LREG x7, 7*REGBYTES(sp)
LREG x8, 8*REGBYTES(sp)
LREG x9, 9*REGBYTES(sp)
LREG x10, 10*REGBYTES(sp)
LREG x11, 11*REGBYTES(sp)
LREG x12, 12*REGBYTES(sp)
LREG x13, 13*REGBYTES(sp)
LREG x14, 14*REGBYTES(sp)
LREG x15, 15*REGBYTES(sp)
LREG x16, 16*REGBYTES(sp)
LREG x17, 17*REGBYTES(sp)
LREG x18, 18*REGBYTES(sp)
LREG x19, 19*REGBYTES(sp)
LREG x20, 20*REGBYTES(sp)
LREG x21, 21*REGBYTES(sp)
LREG x22, 22*REGBYTES(sp)
LREG x23, 23*REGBYTES(sp)
LREG x24, 24*REGBYTES(sp)
LREG x25, 25*REGBYTES(sp)
LREG x26, 26*REGBYTES(sp)
LREG x27, 27*REGBYTES(sp)
LREG x28, 28*REGBYTES(sp)
LREG x29, 29*REGBYTES(sp)
LREG x30, 30*REGBYTES(sp)
LREG x31, 31*REGBYTES(sp)

mret

.p2align 0x2
.section .data 
_data1:
.word 	0
.word 	0
.word 	0
.word 	0

interrupt_handler:
	li t3, 0x0C001010    #plic claim base address
	lw t4, 0(t3)
	li s2, 0x00011300
uarts:	lb s1, 12(s2)
	andi s1,s1,0x2
	bnez s1, uarts
	andi s1,s1,0
	add s1,s1,t4          // print int id
	sb s1, 4(s2)
	sw t4, 0(t3)            // int complete
	ebreak
	


j interrupt_handler
# check for mcause, it is 7 mach plic
# claim interrupt
# use interrupt id and jump to isr, isr will return control back
# complete interrupt


putchar:
	li s2, 0x00011300
uart_status:	lb s1, 12(s2)
	andi s1,s1,0x2
	bnez s1, uart_status
	andi s1,s1,0
	addi s1,s1,66
	sb s1, 4(s2)
	j returning
