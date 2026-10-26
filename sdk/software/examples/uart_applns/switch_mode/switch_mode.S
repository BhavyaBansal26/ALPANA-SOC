/***************************************************************************
* Project               	: shakti devt board
* Name of the file	        : switch_mode.S
* Brief Description of file     : Demonstrates the working of supervisor mode 
* Name of Author    	        : Sathya narayanan
* Email ID                      : sathya281@gmail.com

 Copyright (C) 2020  IIT Madras. All rights reserved.

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
 * @file switch_mode.S
 * @brief Demonstrates the working of supervisor mode 
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

#ifdef ARTIX7_100T
_start:	
	li sp, 0x80001000
	la t0, trap_entry
	csrw mtvec, t0
	li      t0, 0x1800
	csrrc   zero,mstatus, t0 
	li      t0, 0x0800
	csrs    mstatus, t0          # set mpp (previous mode) with supervisor mode
	la  t0, s_mode_begin         # set up mepc with address of supervisor mode re-entry function
	csrw    mepc, t0
	mret                         #exiting M mode 
 

s_mode_begin:
	la x17, _data1  #base address
	sw x17, 0x4(x17) # Store word - Storing Word 
	bnez x10, _start

trap_handler:
	csrr t0,mcause
	li t3, 0x80000000
	and t0,t0,t3
	beqz t0, exception_handler
1:      ret


exception_handler:
	csrr t0,mcause
	la t1, _data1  #base address
	lw t2, 0(t1)  # offset value
	addi t2,t2,4 #t1 holds base address for storing trap occurences
	sw t2,0(t1)  
	addw t1, t1,t2
	sw t0,0(t1)  
	j 1b

.p2align 2
trap_entry:     # currently trap_entry has a nop. Later we will add actual instrns
addi sp, sp, -32*8
 #x0 is always 0
sd x1, 1*8(sp)
sd x2, 2*8(sp)
sd x3, 3*8(sp)
sd x4, 4*8(sp)
sd x5, 5*8(sp)
sd x6, 6*8(sp)
sd x7, 7*8(sp)
sd x8, 8*8(sp)
sd x9, 9*8(sp)
sd x10, 10*8(sp)
sd x11, 11*8(sp)
sd x12, 12*8(sp)
sd x13, 13*8(sp)
sd x14, 14*8(sp)
sd x15, 15*8(sp)
sd x16, 16*8(sp)
sd x17, 17*8(sp)
sd x18, 18*8(sp)
sd x19, 19*8(sp)
sd x20, 20*8(sp)
sd x21, 21*8(sp)
sd x22, 22*8(sp)
sd x23, 23*8(sp)
sd x24, 24*8(sp)
sd x25, 25*8(sp)
sd x26, 26*8(sp)
sd x27, 27*8(sp)
sd x28, 28*8(sp)
sd x29, 29*8(sp)
sd x30, 30*8(sp)
sd x31, 31*8(sp)

jal trap_handler

ld x1, 1*8(sp)
ld x2, 2*8(sp)
ld x3, 3*8(sp)
ld x4, 4*8(sp)
ld x5, 5*8(sp)
ld x6, 6*8(sp)
ld x7, 7*8(sp)
ld x8, 8*8(sp)
ld x9, 9*8(sp)
ld x10, 10*8(sp)
ld x11, 11*8(sp)
ld x12, 12*8(sp)
ld x13, 13*8(sp)
ld x14, 14*8(sp)
ld x15, 15*8(sp)
ld x16, 16*8(sp)
ld x17, 17*8(sp)
ld x18, 18*8(sp)
ld x19, 19*8(sp)
ld x20, 20*8(sp)
ld x21, 21*8(sp)
ld x22, 22*8(sp)
ld x23, 23*8(sp)
ld x24, 24*8(sp)
ld x25, 25*8(sp)
ld x26, 26*8(sp)
ld x27, 27*8(sp)
ld x28, 28*8(sp)
ld x29, 29*8(sp)
ld x30, 30*8(sp)
ld x31, 31*8(sp)

mret

.p2align 0x2
.section .data 
_data1:
.word 	0
.word 	0
.word 	0
.word 	0
#endif
