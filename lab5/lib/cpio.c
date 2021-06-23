#include "utils.h"
#include "uart.h"
#include "thread.h"
#include "memory.h"

//#define CPIO_ADDR 0x20000000 // for pi
#define CPIO_ADDR 0x8000000 // for qemu
#define NULL      ((void *)0)

int ls(char *input, int readfile) {
    /*
    New ASCII Format
    struct cpio_newc_header {
        char	   c_magic[6];
        char	   c_ino[8];
        char	   c_mode[8];
        char	   c_uid[8];
        char	   c_gid[8];
        char	   c_nlink[8];
        char	   c_mtime[8];
        char	   c_filesize[8];
        char	   c_devmajor[8];
        char	   c_devminor[8];
        char	   c_rdevmajor[8];
        char	   c_rdevminor[8];
        char	   c_namesize[8];
        char	   c_check[8];
    };
    */
    // 0x8000000 for qemu, 0x20000000 for raspi 3
    unsigned char *kernel = (unsigned char *) CPIO_ADDR;
    unsigned char *filename;
    unsigned char *filedata;
    int header_offset = 0x6E;
    int filesize_offset = 6 + 6*8;
    int namesize_offset = 6 + 11*8;
    int filesize;
    int namesize;

    while (1) {
        // compute file size and name size
        filesize = 0;
        namesize = 0;
        for (int i = 0; i < 8; i++) {
            // 10~16
            if (kernel[filesize_offset+i] >= 'A') {
                filesize = filesize * 0x10 + (kernel[filesize_offset+i] - 'A' + 0xA);
            }
            // 0~9
            else {
                filesize = filesize * 0x10 + (kernel[filesize_offset+i] - '0');
            }

            if (kernel[namesize_offset+i] >= 'A') {
                namesize = namesize * 0x10 + (kernel[namesize_offset+i] - 'A' + 0xA);
            }
            else {
                namesize = namesize * 0x10 + (kernel[namesize_offset+i] - '0');
            }
        }
        namesize += header_offset;

        // padding
        if (filesize % 4 != 0) {
            filesize += (4 - filesize % 4);
        }
        if (namesize % 4 != 0) {
            namesize += (4 - namesize % 4);
        }

        // check if pathname is "TRAILER!!!"
        filename = kernel + header_offset;
        if (!strcmp(filename, "TRAILER!!!")) return 0;

        // print filename and ignore root
        if (strcmp(filename, ".") && !readfile) {
            uart_puts(filename);
            uart_puts("\n");
        }
        // print file data
        else if (!strcmp(filename, input) && readfile) {
            filedata = kernel + namesize;
            for (int i = 0; i < filesize; i++) {
                uart_send(filedata[i]);
                if (filedata[i] == '\n') uart_send('\r');
            }
            uart_puts("\n");
            return 1;
        }
        kernel += (filesize + namesize);
    }
}
/*
void load_prog(char *path) {
    unsigned char *kernel = (unsigned char *) CPIO_ADDR;
    unsigned char *filename;
    unsigned char *filedata;
    int header_offset = 0x6E;
    int filesize_offset = 6 + 6*8;
    int namesize_offset = 6 + 11*8;
    int filesize;
    int namesize;

    while (1) {
        // compute file size and name size
        filesize = 0;
        namesize = 0;
        for (int i = 0; i < 8; i++) {
            // 10~16
            if (kernel[filesize_offset+i] >= 'A') {
                filesize = filesize * 0x10 + (kernel[filesize_offset+i] - 'A' + 0xA);
            }
            // 0~9
            else {
                filesize = filesize * 0x10 + (kernel[filesize_offset+i] - '0');
            }

            if (kernel[namesize_offset+i] >= 'A') {
                namesize = namesize * 0x10 + (kernel[namesize_offset+i] - 'A' + 0xA);
            }
            else {
                namesize = namesize * 0x10 + (kernel[namesize_offset+i] - '0');
            }
        }
        namesize += header_offset;

        // padding
        if (filesize % 4 != 0) {
            filesize += (4 - filesize % 4);
        }
        if (namesize % 4 != 0) {
            namesize += (4 - namesize % 4);
        }

        // check if pathname is "TRAILER!!!"
        filename = kernel + header_offset;
        if (!strcmp(filename, "TRAILER!!!")) return 0;

        // exe user program
        else if (!strcmp(filename, path)) {
            filedata = kernel + namesize;
            //for (int i = 0; i < filesize; i++) {
            //    addr[i] = filedata[i];
            //}
            uart_puts("loading program...\n");
            //asm volatile("mov x0, 0x3c0"); //disable interrupt
            asm volatile("mov x0, 0x340"); //enable interrupt
            //uart_puts("loading program...\n");
            asm volatile("msr spsr_el1, x0");
            //uart_puts("loading program...\n");
            asm volatile("msr elr_el1, %0"::"r"((unsigned long)filedata));
            //uart_puts("loading program...\n");
            asm volatile("msr sp_el0, %0"::"r"((unsigned long)CPIO_ADDR));
            //uart_puts("loading program...last\n");
            //asm volatile("eret");
            //uart_puts("loading program...last\n");

            // core_timer_enable
            asm volatile("mov x0, 1");
            asm volatile("msr cntp_ctl_el0, x0"); // enable
            asm volatile("mrs x0, cntfrq_el0");
            asm volatile("msr cntp_tval_el0, x0"); // set expired time
            asm volatile("mov x0, 2");
            asm volatile("ldr x1, =0x40000040");
            asm volatile("str w0, [x1]"); // unmask timer interrupt

            asm volatile("eret");
            return 1;
        }
        kernel += (filesize + namesize);
    }
}
*/

unsigned long argvPut(char** argv,unsigned long ret){
	int cnt1=0,cnt2=0;
	for(int i=0;;++i){
		cnt1++;//with null
		if(!argv[i])break;

		for(int j=0;;++j){
			cnt2++;//with null
			if(!argv[i][j])break;
		}
	}

	int sum=8+8+8*cnt1+cnt2;
	ret=(ret-sum);
	//alignment
	ret=ret-(ret&15);

	char* tmp=(char*)ret;
	*(unsigned long*)tmp=cnt1-1;
	tmp+=8;
	*(unsigned long*)tmp=(unsigned long)(tmp+8);
	tmp+=8;
	char* buffer=tmp+8*cnt1;
	for(int i=0;i<cnt1;++i){
		if(i+1==cnt1){
			*(unsigned long*)tmp=0;
		}else{
			*(unsigned long*)tmp=(unsigned long)buffer;
			tmp+=8;
			for(int j=0;;++j){
				*buffer=argv[i][j];
				buffer++;
				if(!argv[i][j])break;
			}
		}
	}
	return ret;
}


void argvPassing(char** argv, unsigned long sp) {
    int argc = 0;
    int argv_size[10]; // max argc = 10
    for (int i = 0;; i++) {
        if (!argv[i]) break;
        argc++;
    }
    for (int i = 0; i < argc; i++) {
        argv_size[i] = 0;
        for (int j = 0;; j++) {
            argv_size[i]++; // with null
            if (!argv[i][j]) break;
        }
    }
    unsigned long *argc_ptr = (unsigned long *)sp;
    argc_ptr[1] = argc;
    argc_ptr = argc_ptr + 1;
    unsigned long *argv_ptr = (unsigned long *)argc_ptr;
    int index = 0;
    for (int i = 0; i < argc; i++) {
        argv_ptr[i] = (unsigned long)(argv_ptr + 1); 
        index++;
    }
    argv_ptr[index] = 0; // NULL
    argv_ptr = argv_ptr + index + 1;
    
    char **argv_data = (char **)argv_ptr;
    int i = 0;
    int j = 0;
    for (i = 0; i < argc; i++) {
        j = 0;
        for (j = 0; j < argv_size[i]-1; j++) {
            argv_data[i][j] = argv[i][j];
        }
        argv_data[i][j+1] = '\0';
    }
}


void load_prog(char *name, char **argv, Task *current) {
    char *path = name;
    unsigned char *kernel = (unsigned char *) CPIO_ADDR;
    unsigned char *filename;
    unsigned char *filedata;
    int header_offset = 0x6E;
    int filesize_offset = 6 + 6*8;
    int namesize_offset = 6 + 11*8;
    int filesize;
    int namesize;

    while (1) {
        // compute file size and name size
        filesize = 0;
        namesize = 0;
        for (int i = 0; i < 8; i++) {
            // 10~16
            if (kernel[filesize_offset+i] >= 'A') {
                filesize = filesize * 0x10 + (kernel[filesize_offset+i] - 'A' + 0xA);
            }
            // 0~9
            else {
                filesize = filesize * 0x10 + (kernel[filesize_offset+i] - '0');
            }

            if (kernel[namesize_offset+i] >= 'A') {
                namesize = namesize * 0x10 + (kernel[namesize_offset+i] - 'A' + 0xA);
            }
            else {
                namesize = namesize * 0x10 + (kernel[namesize_offset+i] - '0');
            }
        }
        namesize += header_offset;

        // padding
        if (filesize % 4 != 0) {
            filesize += (4 - filesize % 4);
        }
        if (namesize % 4 != 0) {
            namesize += (4 - namesize % 4);
        }

        // check if pathname is "TRAILER!!!"
        filename = kernel + header_offset;
        if (!strcmp(filename, "TRAILER!!!")) {
            uart_printf("invalid file path!!\n");
        }

        // exe user program
        else if (!strcmp(filename, path)) {
            filedata = kernel + namesize;
            //for (int i = 0; i < filesize; i++) {
            //    addr[i] = filedata[i];
            //}

            current->addr = malloc(filesize);
            current->size = filesize;

            uart_printf("app size: %d bytes\n", filesize);
            //uart_printf("start at 0x%x\n", (unsigned long)filedata);

            char* target = (char*)current->addr; 
            for (int i = 0; i < filesize; i++) {
                target[i] = filedata[i];
            }


            unsigned long sp;
            //uart_printf("sp: %x\n", sp);
            sp = argvPut(argv, current->addr);
            uart_printf("sp: %x\n", sp);
            //argvPassing(argv, sp);

            uart_puts("loading program...\n");
            
            //asm volatile("mov x0, 0x3c0"); //disable interrupt
            asm volatile("mov x0, 0x340"); //enable interrupt
            asm volatile("msr spsr_el1, x0");
            asm volatile("msr elr_el1, %0"::"r"(current->addr));
            asm volatile("msr sp_el0, %0"::"r"(sp));
            

            // core_timer_enable
            
            asm volatile("mov x0, 1");
            asm volatile("msr cntp_ctl_el0, x0"); // enable
            asm volatile("mrs x0, cntfrq_el0");
            asm volatile("msr cntp_tval_el0, x0"); // set expired time
            asm volatile("mov x0, 2");
            asm volatile("ldr x1, =0x40000040");
            asm volatile("str w0, [x1]"); // unmask timer interrupt
            /*            
            asm volatile("mrs x3, sp_el0		\n"::);
	        asm volatile("ldr x0, [x3, 0]		\n"::);
	        asm volatile("ldr x1, [x3, 8]		\n"::);
            */
            //asm volatile("eret");
            /*
            asm volatile("mov x0, 0x340			\n");//enable interrupt
            asm volatile("msr spsr_el1, x0		\n");
            asm volatile("msr elr_el1, %0		\n"::"r"(current->addr));
            asm volatile("msr sp_el0, %0		\n"::"r"(sp));
            */
            asm volatile("mrs x3, sp_el0"::);
            asm volatile("ldr x0, [x3, 0]"::);
            asm volatile("ldr x1, [x3, 8]"::);
            asm volatile("eret");
        }
        kernel += (filesize + namesize);
    }
}