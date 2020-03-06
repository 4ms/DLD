ENTRY(Reset_Handler)

_estack = 0x20020000;

_Min_Heap_Size = 0x100;
_Min_Stack_Size = 0x400;

MEMORY
{
  RAM (xrw)        : ORIGIN = 0x20000000, LENGTH = 128K
  CCMRAM (xrw)     : ORIGIN = 0x10000000, LENGTH = 64K
  FLASH (rx)       : ORIGIN = 0x08008000, LENGTH = 1M-32K
}

SECTIONS
{
  .isr_vector :
  {
    . = ALIGN(16);
    KEEP(*(.isr_vector)) /* Startup code */
    . = ALIGN(16);
  } >FLASH

  /*
  _siitcm = LOADADDR(.itcm_text);
  .itcm_text :
  {
    . = ALIGN(4);
    _sitcm = .;
	*(.itcm_text)
    *(.itcm_text*)
    _eitcm = .;
  } > FLASH
*/

  .flashtext :
  {
    . = ALIGN(16);
    *(.flashtext)            /* Startup code */
    . = ALIGN(16);
  } >FLASH

  .text :
  {
    . = ALIGN(16);
    *(.text)           /* .text sections (code) */
    *(.text.*)          /* .text* sections (code) */
    *(.rodata)                 /* read-only data (constants) */
    *(.rodata*)
    *(.glue_7)         /* glue arm to thumb code */
    *(.glue_7t)        /* glue thumb to arm code */
    *(.eh_frame)

    KEEP (*(.init))
    KEEP (*(.fini))

    . = ALIGN(16);
    _etext = .;        /* define a global symbols at end of code */
  } >FLASH

  .ARM.extab   : { *(.ARM.extab* .gnu.linkonce.armextab.*) } >FLASH
  .ARM : {
    __exidx_start = .;
    *(.ARM.exidx*)
    __exidx_end = .;
  } >FLASH

  .preinit_array     :
  {
    PROVIDE_HIDDEN (__preinit_array_start = .);
    KEEP (*(.preinit_array*))
    PROVIDE_HIDDEN (__preinit_array_end = .);
  } >FLASH
  .init_array :
  {
    PROVIDE_HIDDEN (__init_array_start = .);
    KEEP (*(SORT(.init_array.*)))
    KEEP (*(.init_array*))
    PROVIDE_HIDDEN (__init_array_end = .);
  } >FLASH
  .fini_array :
  {
    PROVIDE_HIDDEN (__fini_array_start = .);
    KEEP (*(SORT(.fini_array.*)))
    KEEP (*(.fini_array*))
    PROVIDE_HIDDEN (__fini_array_end = .);
	_sidata = __fini_array_end;
  } >FLASH

  /* used by the startup to initialize data */
  /* _sidata = LOADADDR(.data); */

  /* Initialized data sections goes into RAM, load LMA copy after code */
  .data : AT ( _sidata )
  {
    . = ALIGN(16);
    _sdata = .;        /* create a global symbol at data start */
    *(.ramtext*)
    . = ALIGN(16);
    *(.data)           /* .data sections */
    *(.data.*)          /* .data* sections */
    . = ALIGN(16);
    _edata = .;        /* define a global symbol at data end */
  } >RAM

  
  .bss :
  {
  	. = ALIGN(16);
    _sbss = .;
    __bss_start__ = _sbss;
    *(.bss)
    *(.bss*)
    *(COMMON)
    . = ALIGN(16);
    _ebss = .;
    __bss_end__ = _ebss;
  } >RAM

  PROVIDE ( end = _ebss );
  PROVIDE ( _end = _ebss );

  ._user_heap_stack :
  {
    . = ALIGN(16);
    PROVIDE ( end = . );
    PROVIDE ( _end = . );
    . = . + _Min_Heap_Size;
    . = . + _Min_Stack_Size;
    . = ALIGN(16);
  } >RAM

  /* CCM section, vars must be located here explicitly */
  /* Example: int foo __attribute__ ((section (".ccmdata"))); */
  .ccmdata (NOLOAD) :
  {
    . = ALIGN(16);
    *(.ccmdata)
    . = ALIGN(16);
  } >CCMRAM

  /* Remove information from the standard libraries */
  /*
  DISCARD :
  {
    libc.a ( * )
    libm.a ( * )
    libgcc.a ( * )
  }
*/
  .ARM.attributes 0 : { *(.ARM.attributes) }
}


