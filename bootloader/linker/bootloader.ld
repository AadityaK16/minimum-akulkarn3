OUTPUT_ARCH(arm)
ENTRY(_start)

MEMORY
{
  BOOTROM (rx) : ORIGIN = 0x00000000, LENGTH = 64K
}

SECTIONS
{
  .vectors :
  {
    KEEP(*(.vectors))
  } > BOOTROM

  .text :
  {
    *(.text.startup)
    *(.text*)
    *(.glue_7)
    *(.glue_7t)
    *(.vfp11_veneer)
    *(.v4_bx)
  } > BOOTROM

  .rodata :
  {
    *(.rodata*)
  } > BOOTROM

  .iplt : { *(.iplt) } > BOOTROM
  .rel.iplt : { *(.rel.iplt) } > BOOTROM
  .igot.plt : { *(.igot.plt) } > BOOTROM
  .data : { *(.data*) } > BOOTROM
  .bss : { *(.bss*) *(COMMON) } > BOOTROM

  . = ALIGN(4);
  __bootloader_end = .;

  /DISCARD/ :
  {
    *(.ARM.attributes)
    *(.comment)
    *(.note*)
  }
}

ASSERT(__bootloader_end <= ORIGIN(BOOTROM) + LENGTH(BOOTROM), "bootloader overflow")
ASSERT(SIZEOF(.data) == 0, "boot ROM must not contain writable data")
ASSERT(SIZEOF(.bss) == 0, "boot ROM must not contain BSS")
