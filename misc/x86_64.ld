OUTPUT_FORMAT(elf64-x86-64)

ENTRY(kt_main)

PHDRS {
    requests PT_LOAD;
    text     PT_LOAD    FLAGS(0x05);
    rodata   PT_LOAD    FLAGS(0x04);
    data     PT_LOAD    FLAGS(0x06);
    dynamic  PT_DYNAMIC FLAGS(0x06);
}

SECTIONS {
    . = 0xffffffff80000000;

  .requests : {
    KEEP(*(.requests_start))
    KEEP(*(.requests))
    KEEP(*(.requests_end)) 
  } : requests

  . = ALIGN(CONSTANT(MAXPAGESIZE));

  .text : {
    text_start = .;
    *(.text .text.*)
    text_end = .;
  } : text

  . = ALIGN(CONSTANT(MAXPAGESIZE));

  .rodata : {
    *(.rodata .rodata.* .srodata .srodata.*)
    . = ALIGN(8);
    __ktsymbol_start = .;
    KEEP(*(.ktsymbols))
    __ktsymbol_end = .;
  } : rodata
  .dynstr : { *(.dynstr) } :rodata
  .dynsym : { *(.dynsym) } :rodata
  .rela.dyn : { *(.rela.dyn) } :rodata

  . = ALIGN(CONSTANT(MAXPAGESIZE));

  .dynamic : { *(.dynamic) } :data :dynamic
  .got : { *(.got) } :data
  .got.plt : { *(.got.plt) } :data
  .data : {
    *(.data .data.* .sdata .sdata.*)
  } :data

  .bss : {
    bss_start = .;
    *(.bss .bss.*)
    *(COMMON)
    bss_end = .;
    kernel_end = .;
  } : data


  /DISCARD/ : {
    *(.eh_frame*)
    *(.note .note.*)
    *(.interp)
  }
  
}
