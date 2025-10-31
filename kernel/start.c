#include "csr.h"
#include "uart.h"

void start_setup();

/* to be found in kernel.c*/
void kernel_main();

/* jump from entry.S in assembly, to C code  */
/* executes still at Machine Privilege level */
void start_setup()
{
	/* prepare registers to jump into kernel_main in Supervisor Privilege Mode at the end at mret instruction */

	/* SET FIELD MSTATUS:MPP TO S, WHILE PRESERVING OTHER VALUES */
	uint32_t curr_mstatus = READ_CSR("mstatus");
	curr_mstatus = curr_mstatus & (~MSTATUS_MPP_MASK);
	curr_mstatus = curr_mstatus | MSTATUS_MPP_S;
	WRITE_CSR("mstatus", curr_mstatus);

	/* Configure PMP,  give access to all memory in supervisor mode, physical address space is 34 bits  so valid address < 0x4_0000_0000 */
	WRITE_CSR("pmpaddr0", 0xFFFFFFFF);
	/* Top Of Range Mode for pmpaddr0 with A+R+W+X rights set*/
	/* A = 01 X = 1, W = 1, R = 1 */
	/* ??? 01 1 1 1 = 0x0F*/
	/* if pmpaddr0 has pmpcfg0 set to TOR mode, then lower addres is set to 0 */
	/* so any address < pmpaddr0 is accessible in S/U-Mode */

	/* TODO: Change to other configuration mode than TOR, because now it leaves 4 bytes inaccessible on top of the memory */
	WRITE_CSR("pmpcfg0", 0x0F);

	/* delegate all interrupts and exceptions to be handled in supervisor mode */
	/* single bit ON for every delegated exception/interrupt (bit No. == exception/interrupt No.)*/
	WRITE_CSR("medeleg", 0xFFFFFFFF);
	WRITE_CSR("mideleg", 0xFFFFFFFF);

	/* Supervisor external, timer and software Interrupt ENABLE*/
	WRITE_CSR("sie", READ_CSR("sie") | SIE_SEIE | SIE_SSIE | SIE_STIE);

	/* address to jump to mret => going from machine to supervisor mode at the end */
	WRITE_CSR("mepc", (addr_t)kernel_main);

	/* initially disable paging in supervisor mode */
	WRITE_CSR("satp", 0);

	/* TODO: Setup timers */
	/* TODO: handle multiple HARTs */

	/* jump to kernel_main in supervisor mode */
	__asm__ __volatile__("mret");
}
