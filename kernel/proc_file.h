#ifndef _PROC_FILE_H_
#define _PROC_FILE_H_

#include "spike_interface/spike_file.h"
#include "util/types.h"
#include "vfs.h"

//
// file operations
//
int do_open(char *pathname, int flags);
int do_read(int fd, char *buf, uint64 count);
int do_write(int fd, char *buf, uint64 count);
int do_lseek(int fd, int offset, int whence);
int do_stat(int fd, struct istat *istat);
int do_disk_stat(int fd, struct istat *istat);
int do_ioctl(int fd, uint64 request, char *data);
char *do_mmap(char *addr, uint64 length, int prot, int flags, int fd, int64 offset);
int do_read_mmap(char *addr, int length, char *buf);
int do_munmap(char *addr, uint64 length);
int do_close(int fd);

int do_opendir(char *pathname);
int do_readdir(int fd, struct dir *dir);
int do_mkdir(char *pathname);
int do_closedir(int fd);

int do_link(char *oldpath, char *newpath);
int do_unlink(char *path);

void fs_init(void);

// data structure that manages all openned files in a PCB
typedef struct proc_file_management_t {
  struct dentry *cwd;  // vfs dentry of current working directory
  struct file opened_files[MAX_FILES];  // opened files array
  int nfiles;  // the number of files opened by a process
} proc_file_management;

proc_file_management *init_proc_file_management(void);

void reclaim_proc_file_management(proc_file_management *pfiles);

#endif
