/***************************************************************************
 * Project           		: shakti devt board
 * Name of the file	     	: init.c
 * Brief Description of file    : source file for system initialization.
 * Name of Author    	        : Sathya Narayanan N & Abhinav Ramnath
 * Email ID                     : sathya281@gmail.com

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

 ****************************************************************************/
/**
@file init.c
@brief source file for system initialization
@detail This is the beginning part of a application beginning.
Different sections are initialized. main function is called. 
uart is initialized. Trap handler is initialized. 
Disable Xip for Aardonyx
*/

#include "traps.h"
#include "plic_driver.h"
#include "clint_driver.h"
#include "log.h"
#include "utils.h"
#include "qspi.h"
#include "platform.h"
#include "defines.h"
#include "uart.h"

extern void init(void);
extern void trap_entry(void);
extern uart_struct *uart_instance[MAX_UART_COUNT];

extern char _stack_end[];
extern char _stack[];
extern char _heap[];
extern char _heap_end[];

char *stack_end=(char *)&_stack_end;
char *stack_start=(char *)&_stack;
char *heap_start=(char *)&_heap;
char *heap_end=(char *)&_heap_end;

/** @fn void section_init()
 * @brief resets the different sections
 * @details Explicitly 0x0 or 0xffffffff is written all the addresses in different "write" sections of memory
 * @warning takes long time. so the caller is diabled as of now
 */
static void section_init(void)
{
#if 0
	/*Enable below code only on need
	 */
	while(heap_start<=heap_end)
	{
		*heap_start=0xff;
		heap_start++;
	}

	while(stack_end>=stack_start)
	{
		*stack_end=0x0;
		stack_end--;
	}
#endif

}

/** @fn void trap_init()
 * @brief Initialize the trap/interrupt callback routines with user defined handler.
 * @details Assign default handler for trap / interrupt that does not have user defined
 *          callback routines"
 */
static void trap_init(void)
{
	log_trace("trap_init entered \n ");

	mcause_interrupt_table[USER_SW_INTERRUPT]        = default_handler;
	mcause_interrupt_table[SUPER_SW_INTERRUPT]       = default_handler;
	mcause_interrupt_table[RESERVED_INTERRUPT0]      = default_handler;
	mcause_interrupt_table[MACH_SW_INTERRUPT]        = default_handler;
	mcause_interrupt_table[USER_TIMER_INTERRUPT]     = default_handler;
	mcause_interrupt_table[SUPER_TIMER_INTERRUPT]    = default_handler;
	mcause_interrupt_table[RESERVED_INTERRUPT1]      = default_handler;
	mcause_interrupt_table[MACH_TIMER_INTERRUPT]     = mach_clint_handler;
	mcause_interrupt_table[USER_EXT_INTERRUPT]       = default_handler;
	mcause_interrupt_table[SUPERVISOR_EXT_INTERRUPT] = default_handler;
	mcause_interrupt_table[RESERVED_INTERRUPT2]      = default_handler;
	mcause_interrupt_table[MACH_EXTERNAL_INTERRUPT]  = mach_plic_handler;
	mcause_interrupt_table[RESERVED_INTERRUPT3]      = default_handler;
	mcause_interrupt_table[RESERVED_INTERRUPT4]      = default_handler;
	mcause_interrupt_table[RESERVED_INTERRUPT5]      = default_handler;
	mcause_interrupt_table[RESERVED_INTERRUPT6]      = default_handler;

	mcause_trap_table[INSTRUCTION_ADDRESS_MISALIGNED] =
		default_handler;
	mcause_trap_table[INSTRUCTION_ACCESS_FAULT] =
		default_handler;
	mcause_trap_table[ILLEGAL_INSTRUCTION] =
		default_handler;
	mcause_trap_table[BREAKPOINT] =
		default_handler;
	mcause_trap_table[LOAD_ADDRESS_MISALIGNED] =
		default_handler;
	mcause_trap_table[LOAD_ACCESS_FAULT] =
		default_handler;
	mcause_trap_table[STORE_AMO_ADDRESS_MISALIGNED] =
		default_handler;
	mcause_trap_table[STORE_AMO_ACCESS_FAULT] =
		default_handler;
	mcause_trap_table[ENVIRONMENT_CALL_FROM_U_MODE] =
		default_handler;
	mcause_trap_table[ENVIRONMENT_CALL_FROM_S_MODE] =
		default_handler;
	mcause_trap_table[RESERVED_TRAP1] =
		default_handler;
	mcause_trap_table[ENVIRONMENT_CALL_FROM_M_MODE] =
		default_handler;
	mcause_trap_table[INSTRUCTION_PAGE_FAULT] =
		default_handler;
	mcause_trap_table[LOAD_PAGE_FAULT] =
		default_handler;
	mcause_trap_table[RESERVED_TRAP2] =
		default_handler;
	mcause_trap_table[STORE_AMO_PAGE_FAULT] =
		default_handler;

	log_trace("trap_init exited \n ");
}

/** @fn void init(void)
 * @brief initialize the necessary variables for system start
 */
#ifdef ALPANA_SOC
/** @fn static void alpana_park(void)
 * @brief ALPANA: after main() returns, spin in on-chip boot RAM.
 * @details The debugger cannot halt the core while it keeps fetching from
 *          DDR2, but it can halt it in boot RAM, so the next program can be
 *          loaded without pressing the reset button.
 */
static void alpana_park(void)
{
	asm volatile ("csrci mstatus, 8");          /* interrupts off */
	asm volatile ("csrw mie, zero");
	*(volatile uint32_t *)(MTIMECMP + 4) = 0xFFFFFFFF;   /* no timer interrupt */
	volatile uint32_t *park = (volatile uint32_t *)BOOTRAM_PARK;
	park[0] = 0x0000006f;                        /* j . */
	asm volatile ("fence; fence.i" ::: "memory");
	((void (*)(void))BOOTRAM_PARK)();
	while (1) ;
}
#endif

#ifdef ALPANA_SOC
extern uint32_t __fast_load[], __fast_start[], __fast_end[], __fastbss_start[], __fastbss_end[];
/** @fn static void alpana_fast_init(void)
 * @brief ALPANA: copy ALPANA_FAST code into the on-chip boot RAM and clear ALPANA_FAST_BSS.
 */
static void alpana_fast_init(void)
{
	volatile uint32_t *s = __fast_load, *d = __fast_start;
	while (d < (volatile uint32_t *)__fast_end) *d++ = *s++;
	for (d = __fastbss_start; d < (volatile uint32_t *)__fastbss_end; ) *d++ = 0;
	asm volatile ("fence; fence.i" ::: "memory");
}
#endif

void init(void)
{
//	section_init(); // uncomment on need basis
#ifdef ALPANA_SOC
	alpana_fast_init();                          /* before anything calls ALPANA_FAST code */
#endif
	uart_init();
#ifdef ALPANA_SOC
	set_baud_rate(uart_instance[0], 115200);     /* boot ROM leaves UART0 at its reset divisor */
#endif

	log_trace("init entered \n ");

#ifdef AARDONYX
	micron_disable_xip_volatile(0,0);
	flashMemInit();
#endif

	trap_init();

	main();

	log_trace("init exited\n");

#ifdef ALPANA_SOC
	alpana_park();
#endif
}
