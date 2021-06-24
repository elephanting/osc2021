typedef struct _task {
    unsigned long kreg[10]; // kernel registers
    unsigned long kfp; // kernel frame pointer
    unsigned long lr; // link register
    unsigned long ksp; // kernel sp
    unsigned long spsr; // core status
    unsigned long elr; // exception return addr
    unsigned long usp; // user sp
    unsigned long tf[31]; // trap frames
	int id; // thread id
	int status; // 0: running, 1: exit, 2: fork, 3: init task
    int is_child;
	unsigned long addr, size, child_id;
	struct _task* next;
    struct _task* parent;
	/*
	task stack:this ~ this+TASKSIZE
	*/
} Task;


int getpid();
int fork();
int exec(char* name, char *argv[]);
void exit();
void threadtest();
//void vfs_test()
void scheduler();

