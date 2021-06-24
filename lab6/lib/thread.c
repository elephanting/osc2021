#include <uart.h>
#include <memory.h>
#include <utils.h>
#include <thread.h>
#include <cpio.h>

#define NULL                ((void *)0)
#define TASKSIZE            4096

typedef struct _queue {
    Task *begin;
    Task *end;
} Queue; 

static int counter = 0;
static Queue runQueue;
static Queue waitQueue;
static Queue zombie;
static Task *init;

Task* createThread(void *fn);
void scheduler();

void idle() {
    while(1) {
        kill_zombies(); // reclaim threads marked as DEAD
        _fork();
        scheduler(); // switch to any other runnable thread
    }
}

int getpid() {
    /*
    Get current process’s id.
    */
    Task *current;
    asm volatile("mrs %0, tpidr_el1":"=r"(current):);
    return current->id;
}

int exec(char* name, char *argv[]) {
    Task *current;
    asm volatile("mrs %0, tpidr_el1":"=r"(current):);
    load_prog(name, argv, current);
}

void exit() {
    /*
    Terminate the current process.
    */
    Task *current;
    asm volatile("mrs %0, tpidr_el1":"=r"(current):);
    current->status = 1;
    idle();
}
/*
void fork() {
    Task *parent;
    unsigned long x30;
    //asm volatile("mov %0, x30":"=r"(x30):);
    //uart_printf("hiii: %x\n", x30);
    
    asm volatile("mrs %0, tpidr_el1":"=r"(parent):);
    
    //unsigned long x30;
    //asm volatile("mov %0, x30":"=r"(x30):);
    //uart_printf("before: 0x%x\n", x30);
    //uart_printf("after: 0x%x\n", parent->lr);
    asm volatile("mov x0, %0"::"r"(parent));
    asm volatile("stp x19, x20, [x0, 16 * 0]");
    asm volatile("stp x21, x22, [x0, 16 * 1]");
    asm volatile("stp x23, x24, [x0, 16 * 2]");
    asm volatile("stp x25, x26, [x0, 16 * 3]");
    asm volatile("stp x27, x28, [x0, 16 * 4]");
    asm volatile("stp x29, x30, [x0, 16 * 5]"); // x29: fp, x30: lr
    asm volatile("mov x9, sp");
    asm volatile("str x9, [x0, 16 * 6]");
    
    
    //asm volatile("mov %0, x30":"=r"(x30):);
    //uart_printf("hiii: %x\n", x30);

    Task *child = (unsigned long)parent + 0x300000;//malloc(TASKSIZE);
    char *p = (char *)parent;
    char *c = (char *)child;
    //uart_printf("par: %x\n", (unsigned long)parent);

    // task copy
    for (int i = 0; i < TASKSIZE; i++) {
        c[i] = p[i];
    }
    child->addr = parent->addr + 0x300000;//malloc(parent->size); // malloc program size
    
    long kernel_offset = (long)child - (long)parent;
    long user_offset = (long)child->addr - (long)parent->addr;
    child->kfp += kernel_offset;
    child->ksp += kernel_offset;
    child->elr += user_offset;
    child->usp += user_offset;
    child->tf[29] += user_offset; // fp
    child->tf[30] += user_offset; // link register

    // copy program
    
    char *c_a = (char *)child->addr;
    char *p_a = (char *)parent->addr;
    
    for (int i = 0; i < parent->size; i++) {
        c_a[i] = p_a[i];
    }
    

    // copy user stack
    
    //uart_printf("parent usp: %x\n", parent->usp);
    //uart_printf("parent app addr: %x\n", parent->addr);
    p_a = (char *)parent->usp;
	c_a = (char *)child->usp;
	for (int i = 0, ii = parent->addr + parent->size - (parent->usp); i<ii; ++i) {
        //uart_printf("%d\n", ii);
		*c_a=*p_a;
		p_a++;
		c_a++;
	}
    

    child->id = counter++;
    child->child_id = 0;
    parent->child_id = child->id;
    //uart_printf("lr: 0x%x\n", parent->lr);

    // update scheduler
    if (!runQueue.begin) {
        runQueue.begin = child;
        runQueue.end = child;
        child->next = NULL;
    }
    else {
        Task *tmp = runQueue.end;
        tmp->next = child;
        child->next = NULL;
        runQueue.end = child;
    }

    //unsigned long sp;
    //unsigned long stack_size;
    // context update
    //uart_printf("gg\n");
    //Task *current;
    //asm volatile("mrs %0, tpidr_el1":"=r"(current):);
    //if (current->child_id == 0) {
    
    
    //asm volatile("mov %0, sp":"=r"(sp):);
    //stack_size = (unsigned long)child + TASKSIZE - sp;
    //for (int i = 0; i < stack_size; i--) {
    
    //}
    //Task *current;
    //asm volatile("mrs %0, tpidr_el1":"=r"(current):);
    //uart_printf("after: 0x%x, child id: %d\n", current->lr, current->child_id);
    //Task *current;
    //asm volatile("mrs %0, tpidr_el1":"=r"(current):);
    //return current->child_id;
    //asm volatile("ret");
}
*/

void _fork() {
	Task* parent = runQueue.begin->next;
    if (parent->status == 2) {
        Task* child = createThread(0);
        parent->status = 0;
        parent->child_id = child->id;

        Task* tmp = child->next;
        char* src = (char *)parent;
        char* dst = (char *)child;

        // task copy
        for(int i = 0; i < TASKSIZE; ++i) {
            *dst=*src;
            dst++;
            src++;
        }

        child->id = parent->child_id;
        child->addr = malloc(TASKSIZE);
        child->child_id=0;
        child->next = tmp;

        long kernel_offset = (long)child - (long)parent;
        long user_offset = (long)child->addr - (long)parent->addr;
        child->kfp += kernel_offset;
        child->ksp += kernel_offset;
        child->elr += user_offset;
        child->usp += user_offset;
        child->tf[29] += user_offset;
        child->tf[30] += user_offset;

        src= (char *)parent->usp;
        dst = (char *)child->usp;

        // program copy, including stacks
        for (int i = 0, ii = parent->addr + parent->size - (parent->usp); i<ii; ++i) {
            *dst = *src;
            dst++;
            src++;
        }
    }
}

int fork() {
	runQueue.begin->status = 2;
	scheduler();
	return runQueue.begin->child_id;
}



void foo() {
    Task *current;
    asm volatile("mrs %0, tpidr_el1":"=r"(current):);
    uart_puts("Thread id: ");
    char id[10];
    char hex[10];   
    itoa(current->id, id);
    uart_puts(id);
    uart_puts(", address: 0x");
    dec_hex((unsigned long)current, hex);
    uart_puts(hex);
    uart_puts("\n");
    exit();
}

void kill_zombies() {
    if (runQueue.begin) {
        Task *head = runQueue.begin;
        Task *tmp;
        while (head) {
            tmp = head->next;
            if (head->status == 1) {
                runQueue.begin = head->next;                
                free(head);
            }
            head = tmp;
        }
    }
}

void scheduler() {
	if(!runQueue.begin) {
		uart_printf("wait for new thread...\n");
		while(1) {}
	}
	
	if(runQueue.begin == runQueue.end) {}
		//no other task, all done
	else {
		do {
			runQueue.end->next = runQueue.begin;
			runQueue.end = runQueue.begin;
			runQueue.begin = runQueue.begin->next;
			runQueue.end->next = NULL;
		} while(runQueue.begin->status);//ignore abnormal task

		asm volatile("\
			mov x1, %0\n\
			mrs x0, tpidr_el1\n\
			bl _contextswitch\n\
		"::"r"(runQueue.begin));//only use bl to avoid stack usage
	}
}


Task* createThread(void *fn) {
	Task* new = malloc(TASKSIZE);
	
	new->kfp = (unsigned long)new + TASKSIZE;
	new->lr = (unsigned long)fn;
	new->ksp = (unsigned long)new + TASKSIZE;
	new->id = counter++;
	new->status = 0;
	new->addr = NULL;
    new->size = 0;
    new->child_id = 0;
	new->next = NULL;

	if(runQueue.begin) {
		runQueue.end->next = new;
		runQueue.end = runQueue.end->next;
	}
    else {
		runQueue.begin = runQueue.end = new;
	}

	return new;
}

void context_switch() {
    //asm volatile("mov x1, %0"::"r"(next));
	//asm volatile("mrs x0, tpidr_el1");
    
    //uart_puts("123");
    asm volatile("_contextswitch:");
    //uart_puts("456");
    asm volatile("stp x19, x20, [x0, 16 * 0]"::);
    //uart_puts("456");
    asm volatile("stp x21, x22, [x0, 16 * 1]"::);
    //uart_puts("456");
    asm volatile("stp x23, x24, [x0, 16 * 2]"::);
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
    asm volatile("msr tpidr_el1, x1");
    //uart_puts("456");
    
    //unsigned long tmp;
    //asm volatile("mov %0, x30":"=r"(tmp):);
    //uart_printf("hiii: %x\n", tmp);
    asm volatile("ret");
}

void threadtest() {
    unsigned long tmp;
    for (int i = 0; i < 5; i++) {
        if (i == 0) {
            init = createThread(0);
            asm volatile("msr tpidr_el1, %0"::"r"((unsigned long)init));
            asm volatile("mov sp, %0"::"r"((unsigned long)init+TASKSIZE));
            //asm volatile("mov sp, %0"::"r"((unsigned long)init+TASKSIZE));
        }
        else {
            Task *new = createThread(foo);
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
    idle();

}

void foo2() {
    char* argv[]={"argv_test","-o","arg2",0};
    //Task *current;
    //asm volatile("mrs %0, tpidr_el1":"=r"(current):);
    //uart_printf("foo lr: 0x%x\n", current->lr);
	exec("app1",argv);
}

void foo3() {
    char* argv[]={0};
    //Task *current;
    //asm volatile("mrs %0, tpidr_el1":"=r"(current):);
    //uart_printf("foo lr: 0x%x\n", current->lr);
	exec("test",argv);
}

void threadtest2() {
    init = createThread(0);
    asm volatile("msr tpidr_el1, %0"::"r"((unsigned long)init));
    //asm volatile("mov sp, %0"::"r"((unsigned long)init+TASKSIZE));
    Task *new = createThread(foo2);
    //Task *current;
    //asm volatile("mrs %0, tpidr_el1":"=r"(current):);
    //uart_printf("init lr: 0x%x\n", current->lr);
    idle();
}

void vfs_test() {
    init = createThread(0);
    asm volatile("msr tpidr_el1, %0"::"r"((unsigned long)init));
    //asm volatile("mov sp, %0"::"r"((unsigned long)init+TASKSIZE));
    Task *new = createThread(foo3);
    //Task *current;
    //asm volatile("mrs %0, tpidr_el1":"=r"(current):);
    //uart_printf("init lr: 0x%x\n", current->lr);
    idle();
}
