OUTPUT_ARCH("riscv")
ENTRY(_entry)

SECTIONS
{
	. = 0x80000000;

	__kernel_base = .;
	
	.text : {
		KEEP(*(.text._entry)); /* just to make sure _entry code is first in text area */
		*(.text .text.*);
	}
	
	.rodata : ALIGN(4) {      /* make sure section starts at multiple of word (4 bytes) */
		*(.rodata .rodata.*);
	}
	
	.data : ALIGN(4) {
		*(.data .data.*);
	}

	.shell : ALIGN(4) {
		*(.shell .shell.*);
	}
	
	.bss : ALIGN(4) {
		*(.bss .bss.* .sbss .sbss.*);
	}

	. = ALIGN(4096); /* align kernel addres space and kernel stack to full pages */
	. += 4 * 1024; /* 4 KiB (1 page) kernel stack */
	__kernel_stack = .;

	. = ALIGN(4096); /* make sure we are page aligned here as well */
	__free_ram_begin = .;
	. += 64 * 1024 * 1024;
	__free_ram_end = .;

}
