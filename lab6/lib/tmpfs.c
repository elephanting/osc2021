#include "tmpfs.h"
#include "memory.h"
#include "uart.h"
#include "cpio.h"

#define NULL             ((void *)0)
#define MAX_FILE_LENGTH  10
#define MAX_FILE_SIZE    4092


int tmpfs_Write(file* f, const void* buf, unsigned long len) {
	vnode* node = f->vnode;
	Content* content = (Content *)node->internal;
    //uart_printf("file name %s\n", content->name);

    if (f->f_pos + len > MAX_FILE_SIZE) {
        uart_printf("file size too small!!!\n");
        while (1) {}
    }

    const char* buffer = (const char *)buf;
    for (int i = 0; i < len; ++i) {
        content->filedata[f->f_pos] = buffer[i];
        f->f_pos++;
        content->size++;
    }
    return len;
}

int tmpfs_Read(file* f, void* buf, unsigned long len) {
    //uart_printf("aa\n");
	vnode* node = f->vnode;
    //uart_printf("cc\n");
	Content* content = (Content *)node->internal;
	int size = content->size;
    //uart_printf("bb\n");
    char* buffer=(char *)buf;
    int ret = 0;
    //uart_printf("ee\n");
    for (int i = f->f_pos; i < size; ++i) {
        if (ret < len) {
            buffer[ret++] = content->filedata[i];
        }
        else break;
    }
    f->f_pos += ret;
    return ret;
}

int tmpfs_Create(vnode* dir_node, vnode** target, const char* component_name) {
    Content *root_content = (Content *)dir_node->internal;
	vnode *new_node = dir_node + root_content->size + 1;
    root_content->size += 1;
	new_node->mount = dir_node->mount;
	new_node->v_ops = dir_node->v_ops;
	new_node->f_ops = dir_node->f_ops;
	new_node->internal = malloc(sizeof(Content));

	Content *content = (Content *)new_node->internal;
	content->name = malloc(MAX_FILE_LENGTH);
    int name_idx = 0;
    while (1) {
        if (component_name[name_idx]) {
            content->name[name_idx] = component_name[name_idx];
        }
        else {
            content->name[name_idx] = 0;
            break;
        }
        name_idx++;
    }	
	content->size = 0;
    content->filedata = malloc(MAX_FILE_SIZE);
    *target = new_node;	
	return 0;
}

int tmpfs_Lookup(vnode* dir_node, vnode** target, const char* component_name) {
	Content* content = (Content*)(dir_node->internal);

	for (int i = 1; i < content->size+1; ++i) {
		vnode* file = dir_node + i;
		Content* file_content = (Content *)(file->internal);
		if (!strcmp(file_content->name, component_name)) {
			*target = file;
			return i;
		}
	}
	return -1;
}

void tmpfs_init(vnode *root, mount *mnt) {
    root->mount = mnt;
    Content *tmp;

    unsigned char *kernel = (unsigned char *) CPIO_ADDR;
    unsigned char *filename;
    unsigned char *filedata;
    int header_offset = 0x6E;
    int filesize_offset = 6 + 6*8;
    int namesize_offset = 6 + 11*8;
    int filesize;
    int namesize;
    
    root->internal = malloc(sizeof(Content));
    Content *root_content = (Content *)root->internal;
    //tmp = (Content *)root->internal;
    //uart_printf("size %x\n", (unsigned long)tmp);
    root_content->name = NULL; // the first vnode is root dir, so no file name and file size
    root_content->size = 0;
    root_content->filedata = NULL;
    int index = 1;
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
        filedata = kernel + namesize;
        if (!strcmp(filename, "TRAILER!!!")) {
            tmp = (Content *)root->internal;
            //uart_printf("size %d\n", tmp->size);
            //Content *tmp;
            //for (int i = 0; i < root_content->size; i++) {
            //    tmp = (Content *)((root + i)->internal);
            //    uart_printf("%s\n", tmp->name);
            //}
            return 0;
        }
        //uart_printf("%s\n", filename);
        //uart_printf("%d\n", root_content->size);
        vnode *new_node = root + index;
        new_node->mount = root->mount;
        new_node->v_ops = root->v_ops;
        new_node->f_ops = root->f_ops;
        new_node->internal = malloc(sizeof(Content));
        Content *new_content = (Content *)new_node->internal;
        new_content->name = filename;
        new_content->size = filesize;
        new_content->filedata = filedata;
        
        kernel += (filesize + namesize);
        index++;
        root_content->size++;
        //tmp = (Content *)root->internal;
        //uart_printf("size %x\n", (unsigned long)tmp);
    }
}

int tmpfs_Setup(filesystem* fs, mount* mnt) {
	char *name= malloc(6);
	//char tmp[]="tmpfs";//raspi bug!!
	name[0] = 't';
	name[1] = 'm';
	name[2] = 'p';
	name[3] = 'f';
	name[4] = 's';
	name[5] = 0;
	fs->name = name;
	fs->setup_mount = tmpfs_Setup;
	mnt->root = malloc(4096); // vnode array
	mnt->fs = fs;
	tmpfs_init(mnt->root, mnt);
    //Content *c = (Content *)mnt->root->internal;
    //uart_printf("setup size: %d\n", c->size);
	return 0;
}