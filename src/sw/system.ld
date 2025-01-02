MEMORY {
    RAM (rwx)  : ORIGIN = 0x00000000, LENGTH = 128K
}

SECTIONS {
  __stacktop = ORIGIN(RAM) + LENGTH(RAM);

  . = ORIGIN(RAM);
  .text :
  {
    start*(.text)
    *(.text)
    *(.text.*)
    *(.rodata)
    *(.rodata.*)
  } > RAM
  .data :
  {
    . = ALIGN(4);
    *(.sdata)
    *(.sdata.*)
    *(.data)
    *(.data.*)
  } > RAM
  .bss :
  {
    . = ALIGN(4);
    _bss_start = .;
    *(.sbss)
    *(.sbss.*)
    *(.bss)
    *(.bss.*)
    _bss_end = .;
  } > RAM
}
