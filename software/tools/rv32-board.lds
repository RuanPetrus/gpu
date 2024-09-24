/* OUTPUT_ARCH("riscv") */

ENTRY(_enter)

MEMORY{
	TEXT(X)     : ORIGIN =0x00400000 , LENGTH = 64k
	DATA(RW)    : ORIGIN =0x10010000 , LENGTH = 160k
}

/* Section Definitions */
SECTIONS {
    .text : {
	*(.text.metal.init.enter)
	*(.isr_vector)
	*(.text .text.*)
    } > TEXT

    .data : {
	*(.srodata .srodata.*)
	*(.sdata .sdata.* .sdata2.*)
	*(.rodata rodata.*)
	__global_pointer$ = . + 0x800;
	*(.data .data.*)
	. = ALIGN(4);
    } > DATA

    /* .bss section which is used for uninitialized data */
    .bss : {
	__bss_start = .;
	*(.sbss*)
	*(.bss .bss.*)
	*(COMMON)
	. = ALIGN(4);
	__bss_end  = .;
    } > DATA

    __stack_size = DEFINED(__stack_size) ? __stack_size : 0xc800;

    .stack : {
	__stack_start = .;
	. += __stack_size;
	. = ALIGN(4);
	__stack_pointer$ = .;
	__stack_end = .;
    } > DATA

    _end = . ;
}
