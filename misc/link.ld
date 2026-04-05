OUTPUT_FORMAT(elf64-x86-64)

ENTRY(kt_main)

PHDRS {
  limine_requests PT_LOAD;
  text PT_LOAD;
  rodata PT_LOAD;
  data PT_LOAD;
}

SECTIONS {
  . = 0xffffffff80000000;

  .limine_requests : {
    KEEP(*(.limine_requests_start))
    KEEP(*(.limine_requests))
    KEEP(*(.limine_requests_end)) 
  } : limine_requests

  . = ALIGN(CONSTANT(MAXPAGESIZE));

  .text : {
    text_start = .;
    *(.text .text.*)
    text_end = .;
  } : text

  . = ALIGN(CONSTANT(MAXPAGESIZE));

  .rodata : {
    *(.rodata .rodata.*)
    . = ALIGN(8);
    __ktsymbol_start = .;
    KEEP(*(.ktsymbols))
    __ktsymbol_end = .;
  } : rodata

  .note.gnu.build-id : {
    *(.note.gnu.build-id)
  } : rodata

  . = ALIGN(CONSTANT(MAXPAGESIZE));
 
  .init_array : {
    PROVIDE(__init_array_start = .);
    KEEP(*(SORT_BY_INIT_PRIORITY(.init_array.*) SORT_BY_INIT_PRIORITY(.ctors.*)))
    KEEP(*(.init_array .ctors))
    PROVIDE(__init_array_end = .);
  }

  .data : {
    *(.data .data.*)
    data_end = .;
  } : data

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
  }
}
