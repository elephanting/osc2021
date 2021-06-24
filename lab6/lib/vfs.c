#include "vfs.h"
#include "memory.h"
#include "uart.h"
#include "utils.h"
#include "tmpfs.h"

static mount my_mount;
static filesystem my_filesystem;
static void* my_write_fn;
static void* my_read_fn;

void vfs_ls() {
	vnode *root = my_mount.root;
	Content *root_content = root->internal;

	Content *tmp;
	//uart_printf("size %d\n", root_content->size);
	for (int i = 1; i < root_content->size+1; i++) {
		//uart_printf("123\n");
		tmp = (Content *)((root + i)->internal);
        uart_printf("%s\n", tmp->name);
	}
}

file *vfs_open(const char* filename, int flags){
	vnode *root = my_mount.root;
	vnode *file_node;
	Content *root_content = (Content *)root->internal;
	Content *file_content;
	
	if (flags == 1) {
		vnode *target = root + root_content->size + 1;
		//root_content->size += 1;
		//uart_printf("bbbbb\n");
		tmpfs_Create(root, &target, filename);
		//uart_printf("aaaaa\n");
		//file_content = (Content *)(root + root_content->size)->internal;
		//uart_printf("%s\n", file_content->name);
		file *ret = malloc(sizeof(file));
		ret->vnode = target;
		ret->f_pos = 0;
		ret->f_ops = malloc(sizeof(file_operations));
		ret->f_ops->write = my_write_fn;
		ret->f_ops->read = my_read_fn;
		return ret;
	}
	else {
		int index = 1;
		//uart_printf("root siize %d\n", root_content->size);
		for (int i = 1; i < root_content->size+1; i++) { 
			file_node = root + i;
			file_content = (Content *)file_node->internal;
			//uart_printf("%s\n", file_content->name);
			if (!strcmp(file_content->name, filename)) {
				//uart_printf("eeee\n");
				file *ret = malloc(sizeof(file));
				ret->vnode = file_node;
				ret->f_pos = 0;
				ret->f_ops = malloc(sizeof(file_operations));
				ret->f_ops->write = my_write_fn;
				ret->f_ops->read = my_read_fn;
				return ret;
			}
		}	
	}
	return -1;
}

int vfs_close(file* f) {
	free(f->f_ops);
	free(f);
	f->f_pos = 0;
	return 0;
}

int vfs_write(file* f,const void* buf,unsigned long len){
	return f->f_ops->write(f,buf,len);
}

int vfs_read(file* f,void* buf,unsigned long len){
	return f->f_ops->read(f,buf,len);
}

void vfs_init(void* setup_mount_fn, void* write_fn, void* read_fn){
	int (*setup_mount)(filesystem*, mount*) = setup_mount_fn;
	my_write_fn = write_fn;
	my_read_fn = read_fn;

	setup_mount(&my_filesystem, &my_mount);
	//Content *root_content = (Content *)my_mount.root->internal;
	//uart_printf("my node %d\n", root_content->size);
	uart_printf("%s file system has been setup.\n", my_filesystem.name);
}