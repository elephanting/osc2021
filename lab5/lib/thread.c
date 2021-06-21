#include <uart.h>
#include <memory.h>
#include <utils.h>

#define NULL                ((void *)0)
#define TASKSIZE            4096

typedef struct _task {
    /*
    status: 0:sleep, 1:run
    */
	//unsigned long context[12+1+2+1+31];//kreg+ksp & spsr+elr & usp+ureg
    unsigned long context[2]; // sp, lr
	int id;
	int status;
	unsigned long a_addr,a_size,child;
	struct _task* next;
    struct _task* parent;
	/*
	task stack:this ~ this+TASKSIZE
	*/
} Task;

typedef struct _queue {
	Task *begin;
    Task *end;
} Queue; 

static int counter = 0;
static Queue runQueue;

void idle_fn() {
    while(1) {
        //kill_zombies(); // reclaim threads marked as DEAD
        scheduler(); // switch to any other runnable thread
    }
}

void foo() {
    Task *current;
    asm volatile("mrs %0, tpidr_el1":"=r"(current):);
    uart_puts("Thread id: ");
    char id[10];
    memset(id, 0, 10);   
    itoa(id, current->id);
    uart_puts(id);
    uart_puts("\n");
    scheduler();
}

void scheduler() {
    // choose next task
    Task *next = runQueue.begin;
    int is_idle = 0;
    if (runQueue.begin->id == runQueue.end->id) is_idle = 1;
    if (!is_idle) runQueue.begin = next->next;
    next->next = NULL;

    // get current task structure
    Task *current;
    asm volatile("mrs %0, tpidr_el1":"=r"(current):);
    if (current->status) {
        Task *prev_end = runQueue.end;
        runQueue.end = current;
        prev_end->next = current;
        current->next = NULL;
    }

    // context switch
    context_switch(current, next);
}


Task *createTread(void *fn, int is_idle) {
    Task *new = malloc(TASKSIZE);
    new->id = counter++;
    new->status = 1;
    new->next = NULL;
    if (is_idle) runQueue.begin = new;
    runQueue.end = new;
    new->context[0] = (unsigned long)fn; // lr
    new->context[1] = new + TASKSIZE; // sp
    return new;
}

void context_switch(curr, next) {
    asm volatile("mov x1, %0"::"r"(next));
	asm volatile("mov x0, %0"::"r"(curr));
    asm volatile("stp x19, x20, [x0, 16 * 0]"::);
    asm volatile("stp x21, x22, [x0, 16 * 1]"::);
    asm volatile("stp x23, x24, [x0, 16 * 2]"::);
    asm volatile("stp x25, x26, [x0, 16 * 3]"::);
    asm volatile("stp x27, x28, [x0, 16 * 4]"::);
    asm volatile("stp x29, x30, [x0, 16 * 5]"::); // x29: fp, x30: lr
    asm volatile("mov x9, sp"::);
    asm volatile("str x9, [x0, 16 * 6]"::);

    asm volatile("ldp x19, x20, [x1, 16 * 0]"::);
    asm volatile("ldp x21, x22, [x1, 16 * 1]"::);
    asm volatile("ldp x23, x24, [x1, 16 * 2]"::);
    asm volatile("ldp x25, x26, [x1, 16 * 3]"::);
    asm volatile("ldp x27, x28, [x1, 16 * 4]"::);
    asm volatile("ldp x29, x30, [x1, 16 * 5]"::); // x29: fp, x30: lr
    asm volatile("ldr x9, [x1, 16 * 6]"::);
    asm volatile("mov sp, x9"::);
    asm volatile("msr tpidr_el1, x1"::);
    asm volatile("ret"::);
}


int fork() {
    
}

void threadtest() {
    for(int i = 0; i < 5; i++) {
        if (i == 0) {
            createTread(idle_fn, 1);
        }
        else {
            createTread(foo, 0);
        }
    }
}