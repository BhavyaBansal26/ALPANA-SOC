/***************************************************************************
 * Project           		:  ALPANA_SOC (Nexys 4 DDR, 128 MB DDR2)
 * Name of the file	     	:  link.ld
 * Brief Description of file    :  linker file
 * Name of Author    	        :  Sathya Narayanan N & Abhinav ramnath
 * Email ID                     :  sathya281@gmail.com

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
OUTPUT_ARCH( "riscv" )
ENTRY(_start)
SECTIONS
{
  . = 0x80000000;
  .text.init : { *(.text.init) }
  .tohost ALIGN(0x1000) : { *(.tohost) }
  .text : { *(.text) *(.text.*) }
  .rodata : {
       __rodata_start = .;
       *(.rodata)
       *(.rodata.*)
       *(.gnu.linkonce.r.*)
       __rodata_end = .;
 }
  .sdata : {
    __global_pointer$ = . + 0x800;
    *(.srodata.cst16) *(.srodata.cst8) *(.srodata.cst4) *(.srodata.cst2) *(.srodata*)
    *(.sdata .sdata.* .gnu.linkonce.s.*)
  }
  .sbss : {
    __sbss_start = .;
    *(.sbss)
    *(.sbss.*)
    *(.gnu.linkonce.sb.*)
    __sbss_end = .;
 }
  .data : {
    . = ALIGN(4);
    __data_start = .;
    *(.data)
    *(.data.*)
    *(.gnu.linkonce.d.*)
    __data_end = .;
 }
  .bss : {
       . = ALIGN(4);
       __bss_start = .;
       *(.bss)
       *(.bss.*)
       *(.gnu.linkonce.b.*)
       *(COMMON)
       . = ALIGN(4);
       __bss_end = .;
    }
  .tdata :
  {
    _tls_data = .;
    *(.tdata.begin)
    *(.tdata)
    *(.tdata.end)
    _tls_end = .;
  }
  .tbss :
  {
    *(.tbss)
    *(.tbss.end)
  }
  /* ALPANA: ALPANA_FAST code runs from the free on-chip boot RAM (no cache,
     DDR2 fetch ~48 cycles). It is stored (LMA) in DDR2 right here and copied
     to 0x1C80 by init() before main(). The boot code (0x1000-0x1C24) stays. */
  . = ALIGN(4);
  __fast_load = .;
  .fasttext 0x00001C80 : AT(__fast_load) {
    __fast_start = .;
    *(.fasttext .fasttext.*)
    . = ALIGN(4);
    __fast_end = .;
  }
  .fastbss 0x00002C10 (NOLOAD) : {
    __fastbss_start = .;
    *(.fastbss .fastbss.*)
    . = ALIGN(4);
    __fastbss_end = .;
  }
  ASSERT(__fast_end <= 0x2C00, "ALPANA_FAST code does not fit in boot RAM 0x1C80-0x2BFF: mark fewer functions ALPANA_FAST")
  ASSERT(__fastbss_end <= 0x2FC0, "ALPANA_FAST_BSS data does not fit in boot RAM 0x2C10-0x2FBF")
  . = __fast_load + SIZEOF(.fasttext);
  . = ALIGN(4);
  _end = .;
  /* ALPANA: 128 MB DDR2 at 0x80000000 */
  . = 0x80000000 + 0x08000000 - 0x20;
  _free_space = . - _end;
  _STACK_SIZE = (_free_space * 50) / 100 ;
  _HEAP_SIZE = _free_space - _STACK_SIZE;
    .stack : {
       _stack_end = .;
       __stack_pointer$ = .;
       _stack = . - _STACK_SIZE;
    }
   . = _stack;
   .heap : {
       _heap = . - _HEAP_SIZE;
       _heap_end = .;
    }
}
