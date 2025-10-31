.section .text
.global _entry
.section .text._entry

_entry:
	# set up kernel stack
	la sp, __kernel_stack
	# initial stack top, 1 byte behind __kernel stack
	li t0, 4096
	add sp, sp, t0
	
	# jump out of assembly to start_setup() in C code.
	call start_setup

.section .text
spin:
	j spin /* should never happen, because start_setup shoudln't ever return */

