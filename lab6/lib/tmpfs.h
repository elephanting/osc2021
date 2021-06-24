#include "vfs.h"

#ifndef TMPFS_H
#define TMPFS_H

typedef struct tmpfs_content {
    char *name;
    int size;
    char *filedata;
} Content;
#endif

int tmpfs_Write(file* f, const void* buf, unsigned long len);
int tmpfs_Read(file* f, void* buf, unsigned long len);
int tmpfs_Setup(filesystem* fs, mount* mnt);
int tmpfs_Create(vnode* dir_node, vnode** target, const char* component_name);