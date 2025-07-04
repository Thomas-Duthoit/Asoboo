/*
 * Custom linker script for Asoboo game:
 * - Ensures _start is first in .text so emulator always starts at correct entry
 * - Places code at 0x20030000 to match emulator expectations
 */
SECTIONS
{
  . = 0x20030000;
  .text : {
    KEEP(*(.text._start)) /* Ensure _start is first in .text */
    *(.text*)
  }
  .rodata : { *(.rodata*) }
  .data : { *(.data*) }
  .bss : { *(.bss*) }
}
