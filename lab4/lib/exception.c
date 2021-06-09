#include "uart.h"

void dumpState() {
	unsigned long esr,elr,spsr;
    char str[64];
	asm volatile("mrs %0, esr_el1":"=r"(esr):);
	asm volatile("mrs %0, elr_el1":"=r"(elr):);
	asm volatile("mrs %0, spsr_el1":"=r"(spsr):);
    uart_puts("Exception Return Address: 0x");
    dec_hex(elr, str);
    uart_puts(str);
    uart_puts("\n");
}

void exception_handler() {
	dumpState();
}

void interrupt_handler() {
    asm volatile("mrs x0, cntfrq_el0");
    asm volatile("add x0, x0, x0");
	asm volatile("msr cntp_tval_el0, x0");
	unsigned long cntpct, cntfrq, res;
	asm volatile("mrs %0, cntpct_el0":"=r"(cntpct):);
	asm volatile("mrs %0, cntfrq_el0":"=r"(cntfrq):);

	res = cntpct / cntfrq;
    char str[20];
    itoa(res, str);
	uart_puts("Time Elapsed: ");
    uart_puts(str);
	uart_puts("s\n");
}

void notImplemented() {
	dumpState();
	uart_puts("NotImplemented!!\n");
	while(1) {}
}