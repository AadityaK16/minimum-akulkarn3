OUTPUT_ARCH(arm)
ENTRY(minemu_user_main)

SECTIONS
{
  . = 0x00400000;
  .text : { *(.text*) }
  .rodata : { *(.rodata*) }
  .data : { *(.data*) }
  .bss (NOLOAD) : { *(.bss*) *(COMMON) }
}
