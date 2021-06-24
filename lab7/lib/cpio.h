#ifndef CPIO_H
#define CPIO_H

//#define CPIO_ADDR 0x20000000 // for pi
#define CPIO_ADDR 0x8000000 // for qemu

int ls(char *input, int readfile);
//void load_prog(char *name, char **argv, void *current);
//int load_prog_test();

#endif