#include "inc/syscall.h"

#define O_CREAT 1

int main(int argc, char **argv) {
    //uart_printf("1\n");
    int a = open("hello", O_CREAT);
    //uart_printf("2\n");
    int b = open("world", O_CREAT);
    //uart_printf("3\n");
    write(a, "Hello ", 6);
    //uart_printf("4\n");
    write(b, "World!", 6);
    //uart_printf("5\n");
    close(a);
    close(b);
    b = open("hello", 0);
    a = open("world", 0);
    int sz;
    char buf[100];
    sz = read(b, buf, 100);
    sz += read(a, buf + sz, 100);
    buf[sz] = '\0';
    uart_printf("%s\n", buf); // should be Hello World!
    exit();
}