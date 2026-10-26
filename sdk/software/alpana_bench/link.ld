OUTPUT_ARCH("riscv")
ENTRY(_start)
SECTIONS {
  . = 0x80000000;
  .text.init : { *(.text.init) }
  .text : { *(.text*) }
  .rodata : { *(.rodata*) *(.srodata*) }
  . = ALIGN(16);
  .data : { *(.data*) }
  .sdata : { __global_pointer$ = . + 0x800; *(.sdata*) }
  . = ALIGN(16);
  __bss_start = .;
  .sbss : { *(.sbss*) }
  .bss : { *(.bss*) *(COMMON) }
  . = ALIGN(16);
  __bss_end = .;
  . += 0x8000; _stack_top = .;
  . = ALIGN(4096);
  _heap_start = .;           /* free memory used by the memory stress test */
  ASSERT(_heap_start <= 0x88000000, "program does not fit in 128 MB DDR2")
}
