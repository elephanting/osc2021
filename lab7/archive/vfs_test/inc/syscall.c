#include "syscall.h"
#include "printf.h"

int getpid(){
	long ret;
	asm volatile("\
		svc 1\n\
		mov %0, x0\n\
	":"=r"(ret):);
	return ret;
}

int uart_read(char* buf,int size){
	long ret;
	asm volatile("\
		svc 2\n\
		mov %0, x0\n\
	":"=r"(ret):);
	return ret;
}

int uart_write(char* buf,int size){
	long ret;
	asm volatile("\
		svc 3\n\
		mov %0, x0\n\
	":"=r"(ret):);
	return ret;
}

unsigned int uart_printf(char* fmt,...){
	char dst[100];
    //__builtin_va_start(args, fmt): "..." is pointed by args
    //__builtin_va_arg(args,int): ret=(int)*args;args++;return ret;
    __builtin_va_list args;
    __builtin_va_start(args,fmt);
    unsigned int ret=vsprintf(dst,fmt,args);
	if (ret == 1) while (1) {}
    uart_write(dst,ret);
    return ret;
}

int exec(char* name,char** argv){
	long ret;
	asm volatile("\
		svc 4\n\
		mov %0, x0\n\
	":"=r"(ret):);
	return ret;
}

void exit(){
	asm volatile("svc 5\n"::);
	uart_printf("syscall exit fail\n");
	while (1) {}
}

int fork(){
	long ret;
	asm volatile("\
		svc 6\n\
		mov %0, x0\n\
	":"=r"(ret):);
	return ret;
}

int open(const char *pathname, int flags) {
	int addr;
	asm volatile("\
		svc 7\n\
		mov %0, x0\n\
	":"=r"(addr):);
	return addr;
}
int close(int fd) {
	asm volatile("svc 8");
	return 0;
}

int write(int fd, const void *buf, int count) {
	int addr;
	asm volatile("\
		svc 9\n\
		mov %0, x0\n\
	":"=r"(addr):);
	return addr;
}

int read(int fd, void *buf, int count) {
	int addr;
	asm volatile("\
		svc 10\n\
		mov %0, x0\n\
	":"=r"(addr):);
	return addr;
}