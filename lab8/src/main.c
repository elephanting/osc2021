#include "uart.h"
#include "reboot.h"
#include "utils.h"
#include "cpio.h"
#include "memory.h"
#include "tmpfs.h"
#include "fat.h"
#include "vfs.h"

int main() {
    uart_init();
    char *helloworld = "\n _   _      _ _         __        __         _     _ \n\
| | | | ___| | | ___    \\ \\      / /__  _ __| | __| |\n\
| |_| |/ _ \\ | |/ _ \\    \\ \\ /\\ / / _ \\| '__| |/ _` |\n\
|  _  |  __/ | | (_) |    \\ V  V / (_) | |  | | (_| |\n\
|_| |_|\\___|_|_|\\___( )    \\_/\\_/ \\___/|_|  |_|\\__,_|\n\
                    |/                               \n";
    uart_puts(helloworld);
    char buf[20];
    char *help = "help";
    char *hello = "hello";
    char *reboot = "reboot";
    char cat[4];
    char filename[10];
    allocator_init();
    sd_init();
	vfs_init(fat_Setup);
    
    while(1) {
        uart_puts("#");
        input(buf);
        uart_send('\r');
        
        slice_str(buf, cat, filename, 3);
        if(!strcmp(buf, hello)) {
            uart_puts("Hello World!\n");
        }
        else if(!strcmp(buf, help)) {
            uart_puts("hello: print Hello World!\n");
            uart_puts("help: print all available commands\n");
            uart_puts("reboot: reboot\n");
            uart_puts("ls: filenames in current directory\n");
            uart_puts("cat filename: display the file\n");
        }
        else if(!strcmp(buf, reboot)) {
            reset(1000);
        }
        else if(!strcmp(buf, "thread")) {
            threadtest();
        }
        else if(!strcmp(buf, "thread2")) {
            threadtest2();
        }

        else if (!strcmp(buf, "demo_frame")) {
            allocator_init();
            unsigned long *p1 = malloc(4096*3);
            unsigned long *p2 = malloc(4096);
            unsigned long *p3 = malloc(4096);

            free(p1);
            free(p3);
            free(p2);
        }
        else if (!strcmp(buf, "demo_frame2")) {
            allocator_init();
            unsigned long *p1 = malloc(4096*5);
            unsigned long *p2 = malloc(4096);
            unsigned long *p3 = malloc(4096);

            free(p1);
            free(p3);
            free(p2);
        }
        else if (!strcmp(buf, "demo")) {
            allocator_init();
            unsigned long *p1 = malloc(8);
            unsigned long *p2 = malloc(17);
            unsigned long *p3 = malloc(2000);       

            free(p1);
            free(p2);
            free(p3);
        }
        else if (!strcmp(buf, "demo2")) {
            allocator_init();
            unsigned long *array[300];
            for (int i = 0; i < 300; i++) {
                array[i] = malloc(8);
            }
            for (int i = 0; i < 300; i++) {
                free(array[i]);
            }
        }
        else if (!strcmp(buf, "lab6-1")) {
            char res[100];
            //vfs_ls();
            file *f = vfs_open("create_test", 1);
            vfs_ls();
            //uart_printf("ggggg\n");
            f->f_ops->write(f, "THis is write test\n", 18);
            //uart_printf("dd\n");
            vfs_close(f);
            //uart_printf("ee\n");
            f = vfs_open("create_test", 0);
            //uart_printf("gg\n");
            //f->f_pos = 0;
            f->f_ops->read(f, res, 100);
            uart_printf("%s\n", res);
        }
        else if (!strcmp(buf, "lab6-2")) {
            vfs_test();
        }
        else if (!strcmp(buf, "lab7")) {
            char res[100];
            //vfs_ls();
            file *f = vfs_open("create_test", 1);
            
            //vfs_ls();
            //uart_printf("ggggg\n");
            f->f_ops->write(f, "THis is write test\n", 18);
            //uart_printf("dd\n");
            vfs_close(f);
            //uart_printf("ee\n");
            f = vfs_open("create_test", 0);
            //uart_printf("gg\n");
            //f->f_pos = 0;
            f->f_ops->read(f, res, 100);
            uart_printf("%s\n", res);
        }

        /*
        else if (!strcmp(buf, "ls")) {
            ls("\0", 0);
        }
        else if (!strcmp(cat, "cat")) {
            if(!ls(filename, 1)) {
                uart_puts("Invalid pathname!!\n");
            }
        }
        
        else if (!strcmp(buf, "loadprog")) {
            if (!load_prog("user_prog")) {
                uart_puts("Invalid program name!!\n");
            }
        }

        else if (!strcmp(buf, "test")) {
            if (!load_prog_test()) {
                uart_puts("Invalid program name!!\n");
            }
        }
        */
        
        else {
            uart_puts("Error: ");
            uart_puts(buf);
            uart_puts(" command not found! Try <help> to check all available commands\n");
        }
    }
    return 0;
}