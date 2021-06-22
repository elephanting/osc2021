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
    unsigned long context[12+1]; // sp, lr
	int id;
	int status; // 1: running, 0: waiting, -1: exit, 2: init thread
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
static Queue waitQueue;
static Queue zombie;
static Task *init;

void idle_fn() {
    while(1) {
        kill_zombies(); // reclaim threads marked as DEAD
        scheduler(); // switch to any other runnable thread
    }
}

void foo() {
    Task *current;
    asm volatile("mrs %0, tpidr_el1":"=r"(current):);
    uart_puts("Thread id: ");
    char id[10];
    char hex[10];
    memset(id, 0, 10);   
    itoa(current->id, id);
    uart_puts(id);
    uart_puts(", address: 0x");
    dec_hex((unsigned long)current, hex);
    uart_puts(hex);
    uart_puts("\n");
    current->status = -1;
    scheduler();
}

void kill_zombies() {
    if (zombie.begin) {
        Task *head = zombie.begin;
        Task *next;
        while (head) {
            next = head->next;
            free(head);
            head = next;
        }
        zombie.begin = NULL;
        zombie.end = NULL;
    }
}

void scheduler() {
    Task *current;
    asm volatile("mrs %0, tpidr_el1":"=r"(current):);
    if (!runQueue.begin) {
        if (current->status == 2) return;
        else {
            if (current->status == 0) {
                if (!waitQueue.begin) {
                    waitQueue.begin = current;
                    waitQueue.end = current;
                    current->next = NULL;
                }
                else {
                    Task *prev_end = waitQueue.end;
                    waitQueue.end = current;
                    prev_end->next = current;
                    current->next = NULL;
                }            
            }
            else if (current->status == -1) {
                if (!zombie.begin) {
                    zombie.begin = current;
                    zombie.end = current;
                    current->next = NULL;
                }
                else {
                    Task *prev_end = zombie.end;
                    zombie.end = current;
                    prev_end->next = current;
                    current->next = NULL;
                }
            }
            context_switch(current, init);
        }
    }

    // choose next task
    Task *next = runQueue.begin;
    runQueue.begin = next->next;
    next->next = NULL;
    /*
    char hex[20];
    dec_hex((unsigned long)next, hex);
    uart_puts("0x");
    uart_puts(hex);
    uart_puts("\n");
    */
    // get current task structure

    if (current->status == 1) {
        Task *prev_end = runQueue.end;
        runQueue.end = current;
        prev_end->next = current;
        current->next = NULL;
    }
    else if (current->status == 0) {
        if (!waitQueue.begin) {
            waitQueue.begin = current;
            waitQueue.end = current;
            current->next = NULL;
        }
        else {
            Task *prev_end = waitQueue.end;
            waitQueue.end = current;
            prev_end->next = current;
            current->next = NULL;
        }
    }
    else if (current->status == -1) {
        if (!zombie.begin) {
            zombie.begin = current;
            zombie.end = current;
            current->next = NULL;
        }
        else {
            Task *prev_end = zombie.end;
            zombie.end = current;
            prev_end->next = current;
            current->next = NULL;
        }
    }

    // context switch
    //unsigned long tmp;
    //uart_puts("0x");
    //asm volatile("mov x1, %0"::"r"((unsigned long)next));
	//asm volatile("mrs x0, tpidr_el1"::);
    //asm volatile("add x0, x0, 4096"::);//"r"((unsigned long)TASKSIZE));
    //asm volatile("sub x0, x0, 13*8");
    
    //asm volatile("mov %0, sp":"=r"(tmp):);
    
    //char hex[20];
    //unsigned long *p = (unsigned long)next + 88;
    //dec_hex(*p, hex);
    //uart_puts("0x");
    //uart_puts(hex);
    //uart_puts("\n");
    

    //uart_puts("123");
    //asm("bl _contextswitch");
    context_switch(current, next);
}


Task *createThread(void *fn, int is_init) {
    Task *new = malloc(TASKSIZE);
    new->id = counter++;
    new->status = 1;
    new->next = NULL;
    if (!is_init) {
        if (!runQueue.begin) {
            runQueue.begin = new;
            runQueue.end = new;
        }
        else {
            runQueue.end->next = new;
            runQueue.end = new;
        }
    }
    else {
        new->status = 2;
    }
    memset(new->context, 0, 13);
    new->context[10] = (unsigned long)new + TASKSIZE; //fp
    new->context[11] = (unsigned long)fn; // lr
    new->context[12] = (unsigned long)new + TASKSIZE; // sp
    return new;
}

void context_switch(curr, next) {
    asm volatile("mov x1, %0"::"r"(next));
	asm volatile("mov x0, %0"::"r"(curr));
    //asm volatile("mov x1, %0"::"r"((unsigned long)0x20008000));
    //asm volatile("mrs x0, tpidr_el1"::);
    //unsigned long tmp;
    //char hex[20];
    
    //uart_puts("123");
    //asm volatile("_contextswitch:");
    //uart_puts("456");
    asm volatile("stp x19, x20, [x0, 16 * 0]"::);
    //uart_puts("456");
    asm volatile("stp x21, x22, [x0, 16 * 1]"::);
    //uart_puts("456");
    //asm volatile("stp x23, x24, [x0, 16 * 2]"::);
    //uart_puts("789");
    asm volatile("stp x25, x26, [x0, 16 * 3]"::);
    asm volatile("stp x27, x28, [x0, 16 * 4]"::);
    asm volatile("stp x29, x30, [x0, 16 * 5]"::); // x29: fp, x30: lr
    asm volatile("mov x9, sp"::);
    asm volatile("str x9, [x0, 16 * 6]"::);
    
    //asm volatile("mov %0, x1":"=r"(tmp):);
    //dec_hex(tmp, hex);
    //uart_puts("0x");
    //uart_puts(hex);
    //uart_puts("\n");
    asm volatile("ldp x19, x20, [x1, 16 * 0]"::);
    asm volatile("ldp x21, x22, [x1, 16 * 1]"::);
    asm volatile("ldp x23, x24, [x1, 16 * 2]"::);
    asm volatile("ldp x25, x26, [x1, 16 * 3]"::);
    asm volatile("ldp x27, x28, [x1, 16 * 4]"::);
    asm volatile("ldp x29, x30, [x1, 16 * 5]"::); // x29: fp, x30: lr
    asm volatile("ldr x9, [x1, 16 * 6]"::);
    asm volatile("mov sp, x9"::);
    asm volatile("msr tpidr_el1, x1"::);
    //uart_puts("456");
    
    
    //asm volatile("mov %0, x30":"=r"(tmp):);
    //dec_hex(tmp, hex);
    //uart_puts("0x");
    //uart_puts(hex);
    //uart_puts("\n");
    asm volatile("ret"::);
}


int fork() {
    
}

void threadtest() {
    unsigned long tmp;
    for (int i = 0; i < 5; i++) {
        if (i == 0) {
            init = createThread(idle_fn, 1);
            asm volatile("msr tpidr_el1, %0"::"r"((unsigned long)init));
            asm volatile("mov sp, %0"::"r"((unsigned long)init+TASKSIZE));
            //asm volatile("mov sp, %0"::"r"((unsigned long)init+TASKSIZE));
        }
        else {
            Task *new = createThread(foo, 0);
        }
        /*
        asm volatile("mov %0, sp":"=r"(tmp):);
        char hex[20];
        dec_hex(tmp, hex);
        uart_puts("0x");
        uart_puts(hex);
        uart_puts("\n");
        */
    }

    //asm volatile("mrs %0, tpidr_el1":"=r"((unsigned long)init):);
    //void (*jump)(void) = (unsigned long *)init->context[11];
    //jump();
    /*
    asm volatile("mov x1, %0"::"r"((unsigned long)init));
	asm volatile("mrs x0, tpidr_el1"::);
    //asm volatile("add x0, x0, 4096"::);//"r"((unsigned long)TASKSIZE));
    //asm volatile("sub x0, x0, 13*8");
    asm volatile("mov %0, x0":"=r"(tmp):);
    char hex[20];
    dec_hex(tmp, hex);
    uart_puts("0x");
    uart_puts(hex);
    uart_puts("\n");
    */
    idle_fn();

}