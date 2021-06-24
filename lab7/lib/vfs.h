#ifndef VFS_H
#define VFS_H

//struct _filesystem;

typedef struct _vnode {
  struct mount* mount;
  struct vnode_operations* v_ops;
  struct file_operations* f_ops;
  void* internal;
} vnode;

typedef struct _mount { // done
  vnode* root;
  struct _filesystem* fs;
} mount;

typedef struct _filesystem { // done
  const char* name; 
  int (*setup_mount)(struct _filesystem* fs, mount* mount);
} filesystem;

typedef struct _file {
  vnode* vnode;
  long f_pos; // The next read/write position of this opened file
  struct _file_operations* f_ops;
  int flags;
} file;

typedef struct _file_operations { // done
  int (*write) (file* file, const void* buf, long len);
  int (*read) (file* file, void* buf, long len);
} file_operations;

typedef struct _vnode_operations {
  int (*lookup)(vnode* dir_node, vnode** target, const char* component_name); //done
  int (*create)(vnode* dir_node, vnode** target, const char* component_name); //done
} vnode_operations;


file* vfs_open(const char* filename, int flags); // flags = 0: open, flags = 1: create
int vfs_close(file* f);
int vfs_write(file* f,const void* buf,unsigned long len);
int vfs_read(file* f,void* buf,unsigned long len);
void vfs_init(void* setup_mount_fn, void *write_fn, void *read_fn);
void vfs_ls();
/*
struct mount* rootfs;

int register_filesystem(struct filesystem* fs) {
  // register the file system to the kernel.
}

struct file* vfs_open(const char* pathname, int flags) {
  // 1. Lookup pathname from the root vnode.
  // 2. Create a new file descriptor for this vnode if found.
  // 3. Create a new file if O_CREAT is specified in flags.
}
int vfs_close(struct file* file) {
  // 1. release the file descriptor
}
int vfs_write(struct file* file, const void* buf, size_t len) {
  // 1. write len byte from buf to the opened file.
  // 2. return written size or error code if an error occurs.
}
int vfs_read(struct file* file, void* buf, size_t len) {
  // 1. read min(len, readable file data size) byte to buf from the opened file.
  // 2. return read size or error code if an error occurs.
}
*/
#endif
