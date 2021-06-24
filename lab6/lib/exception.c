#include "uart.h"
#include "thread.h"
#include "vfs.h"

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

void x0Set(unsigned long v){
	Task* current;
	asm volatile("mrs %0, tpidr_el1	\n":"=r"(current):);
	current->tf[0] = v;
}

void exception_handler() {
	unsigned long x0, x1, x2;
	asm volatile("mov %0, x0":"=r"(x0):);
	asm volatile("mov %0, x1":"=r"(x1):);
	asm volatile("mov %0, x2":"=r"(x2):);
	unsigned long esr, svc;
	asm volatile("mrs %0, esr_el1	\n":"=r"(esr):);
	if (((esr>>26) & 0x3f) == 0x15) { // 26: reserved bits, 0x3f: EC bits, 0x15: svc64
		svc = esr & 0x1ffffff; // ISS (Instruction Specific Syndrome)
		//dumpState();
		//uart_printf("svc: %d\n", svc);
		if (svc == 0) {
			dumpState();
			return;
		}
		//getpid
		else if (svc == 1) {
			unsigned long ret = getpid();
			x0Set(ret);
			return;
		}
		//uart_read
		else if (svc == 2) {
			unsigned long ret = uart_getsize((char*)x0, (int)x1);
			x0Set(ret);
			return;
		}
		//uart_write
		else if (svc == 3) {
			uart_puts((char*)x0);
			x0Set(x1);
			return;
		}
		// exec
		else if (svc == 4) {
			exec((char*)x0,(char**)x1);
			x0Set(0);
			return;
		}
		// exit
		else if (svc == 5){
			exit();
			uart_printf("exit fail at exception handler\n");
			while(1) {}
			return;
		}
		// fork
		else if (svc == 6) {
			//Task *current;
    		//asm volatile("mrs %0, tpidr_el1":"=r"(current):);
			//uart_printf("exception lr: 0x%x\n", current->lr);
			//unsigned long ret = fork();
			unsigned long ret = fork();
			x0Set(ret);
			return;
		}
		// open
		else if (svc == 7) {
			//Task *current;
    		//asm volatile("mrs %0, tpidr_el1":"=r"(current):);
			//uart_printf("exception lr: 0x%x\n", current->lr);
			//unsigned long ret = fork();
			unsigned long ret = (unsigned long)vfs_open((const char *)x0, (int)x1);
			x0Set(ret);
			return;
		}
		// close
		else if (svc == 8) {
    		//asm volatile("mrs %0, tpidr_el1":"=r"(current):);
			//uart_printf("exception lr: 0x%x\n", current->lr);
			//unsigned long ret = fork();
			unsigned long ret = vfs_close((file *)x0);
			x0Set(ret);
			return;
		}
		// write
		else if (svc == 9) {
    		//asm volatile("mrs %0, tpidr_el1":"=r"(current):);
			//uart_printf("exception lr: 0x%x\n", current->lr);
			//unsigned long ret = fork();
			unsigned long ret = vfs_write((file *)x0, (const void*)x1, (unsigned long)x2);
			x0Set(ret);
			return;
		}
		// read
		else if (svc == 10) {
			Task *current;
    		//asm volatile("mrs %0, tpidr_el1":"=r"(current):);
			//uart_printf("exception lr: 0x%x\n", current->lr);
			//unsigned long ret = fork();
			unsigned long ret = vfs_read((file *)x0, (void*)x1, (unsigned long)x2);
			x0Set(ret);
			return;
		}
		
	}
	else {
		uart_puts("unknown exception...\n");
		while(1) {}
	}
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
	scheduler();
}

void notImplemented() {
	dumpState();
	uart_puts("NotImplemented!!\n");
	while(1) {}
}