/*
Copyright 2018 Embedded Microprocessor Benchmark Consortium (EEMBC)

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.

Original Author: Shay Gal-on
*/
#include "coremark.h"
#include "core_portme.h"

#if VALIDATION_RUN
volatile ee_s32 seed1_volatile = 0x3415;
volatile ee_s32 seed2_volatile = 0x3415;
volatile ee_s32 seed3_volatile = 0x66;
#endif
#if PERFORMANCE_RUN
volatile ee_s32 seed1_volatile = 0x0;
volatile ee_s32 seed2_volatile = 0x0;
volatile ee_s32 seed3_volatile = 0x66;
#endif
#if PROFILE_RUN
volatile ee_s32 seed1_volatile = 0x8;
volatile ee_s32 seed2_volatile = 0x8;
volatile ee_s32 seed3_volatile = 0x8;
#endif
volatile ee_s32 seed4_volatile = ITERATIONS;
volatile ee_s32 seed5_volatile = 0;
/* Porting : Timing functions
        How to capture time and convert to seconds must be ported to whatever is
   supported by the platform. e.g. Read value from on board RTC, read value from
   cpu clock cycles performance counter etc. Sample implementation for standard
   time.h and windows.h definitions included.
*/
#include "lib.h"
static uint64_t cm_c0, cm_c1, cm_i0, cm_i1;
int cm_errors, cm_short;
CORETIMETYPE
barebones_clock()
{
    return read_csr(mcycle);
}
/* Define : TIMER_RES_DIVIDER
        Divider to trade off timer resolution and total time that can be
   measured.

        Use lower values to increase resolution, but make sure that overflow
   does not occur. If there are issues with the return value overflowing,
   increase this value.
        */
#define GETMYTIME(_t)              (*_t = barebones_clock())
#define MYTIMEDIFF(fin, ini)       ((fin) - (ini))
#define TIMER_RES_DIVIDER          1
#define SAMPLE_TIME_IMPLEMENTATION 1
#define EE_TICKS_PER_SEC           (CPU_HZ / TIMER_RES_DIVIDER)

/** Define Host specific (POSIX), or target specific global time variables. */
static CORETIMETYPE start_time_val, stop_time_val;

/* Function : start_time
        This function will be called right before starting the timed portion of
   the benchmark.

        Implementation may be capturing a system timer (as implemented in the
   example code) or zeroing some system parameters - e.g. setting the cpu clocks
   cycles to 0.
*/
void
start_time(void)
{
    cm_i0 = rdinstret64(); cm_c0 = rdcycle64();
    GETMYTIME(&start_time_val);
}
/* Function : stop_time
        This function will be called right after ending the timed portion of the
   benchmark.

        Implementation may be capturing a system timer (as implemented in the
   example code) or other system parameters - e.g. reading the current value of
   cpu cycles counter.
*/
void
stop_time(void)
{
    GETMYTIME(&stop_time_val);
    cm_c1 = rdcycle64(); cm_i1 = rdinstret64();
}
/* Function : get_time
        Return an abstract "ticks" number that signifies time on the system.

        Actual value returned may be cpu cycles, milliseconds or any other
   value, as long as it can be converted to seconds by <time_in_secs>. This
   methodology is taken to accommodate any hardware or simulated platform. The
   sample implementation returns millisecs by default, and the resolution is
   controlled by <TIMER_RES_DIVIDER>
*/
CORE_TICKS
get_time(void)
{
    CORE_TICKS elapsed
        = (CORE_TICKS)(MYTIMEDIFF(stop_time_val, start_time_val));
    return elapsed;
}
/* Function : time_in_secs
        Convert the value returned by get_time to seconds.

        The <secs_ret> type is used to accommodate systems with no support for
   floating point. Default implementation implemented by the EE_TICKS_PER_SEC
   macro above.
*/
secs_ret
time_in_secs(CORE_TICKS ticks)
{
    secs_ret retval = ((secs_ret)ticks) / (secs_ret)EE_TICKS_PER_SEC;
    return retval;
}

ee_u32 default_num_contexts = 1;

/* Function : portable_init
        Target specific initialization code
        Test for some common mistakes.
*/
void
portable_init(core_portable *p, int *argc, char *argv[])
{

    (void)argc; // prevent unused warning
    (void)argv; // prevent unused warning

    if (sizeof(ee_ptr_int) != sizeof(ee_u8 *))
    {
        ee_printf(
            "ERROR! Please define ee_ptr_int to a type that holds a "
            "pointer!\n");
    }
    if (sizeof(ee_u32) != 4)
    {
        ee_printf("ERROR! Please define ee_u32 to a 32b unsigned type!\n");
    }
    p->portable_id = 1;
}
/* Function : portable_fini
        Target specific final code
*/
void
portable_fini(core_portable *p)
{
    p->portable_id = 0;
    uint64_t cyc = cm_c1 - cm_c0, ins = cm_i1 - cm_i0;
    uint32_t it = (uint32_t)ITERATIONS;
    ee_printf("\n==== COREMARK RESULT (@ %u MHz) ====\n", (unsigned)(CPU_HZ / 1000000u));
    ee_printf(" iterations           : %u\n", it);
    ee_printf(" cycles               : %llu\n", cyc);
    ee_printf(" instructions         : %llu\n", ins);
    ee_printf(" cycles / iteration   : "); print_fix(cyc, it, 0);
    ee_printf("\n CPI                  : "); print_fix(cyc, ins, 3);
    ee_printf("\n IPC                  : "); print_fix(ins, cyc, 3);
    ee_printf("\n MIPS (native)        : "); print_fix(ins * (CPU_HZ / 1000u), cyc * 1000ull, 2);
    ee_printf("\n CoreMark (iter/s)    : "); print_fix((uint64_t)it * CPU_HZ, cyc, 3);
    ee_printf("\n CoreMark / MHz       : "); print_fix((uint64_t)it * 1000000ull, cyc, 4);
    int crc_err = cm_errors - cm_short;
    ee_printf("\n CRC validation       : %s", crc_err ? "FAIL" : "PASS");
    if (cm_short) ee_printf("\n NOTE: run < 10 s, so the score is not an official CoreMark result (raise CM_ITER)");
    ee_printf("\n RESULT: %s\n", crc_err ? "FAIL" : "PASS");
    halt(crc_err != 0);
}
