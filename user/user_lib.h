/*
 * header file to be used by applications.
 */

#ifndef _USER_LIB_H_
#define _USER_LIB_H_
#include "util/types.h"
#include "kernel/proc_file.h"

#include "unistd.h"
#include "fcntl.h"
#include "util/types.h"

int printu(const char *s, ...);
int exit(int code);
void* naive_malloc();
void naive_free(void* va);
int fork();
void yield();

// added @ lab4_1
int open_u(const char *pathname, int flags);
int read_u(int fd, void *buf, uint64 count);
int write_u(int fd, void *buf, uint64 count);
int lseek_u(int fd, int offset, int whence);
int stat_u(int fd, struct istat *istat);
int disk_stat_u(int fd, struct istat *istat);
int close(int fd);

// added @ lab4_2
int opendir_u(const char *pathname);
int readdir_u(int fd, struct dir *dir);
int mkdir_u(const char *pathname);
int closedir_u(int fd);

// added @ lab4_3
int link_u(const char *fn1, const char *fn2);
int unlink_u(const char *fn);

// added @lab5_1
int uartputchar(char ch);
int uartgetchar();
int uart2putchar(char ch);
void car_control(char val);

// added @lab5_3
#define PROT_READ  0x1     // Page can be read.
#define PROT_WRITE 0x2     // Page can be written.
#define PROT_EXEC  0x4     // Page can be executed.
#define PROT_NONE  0x0     // Page can not be accessed.
#define PROT_GROWSDOWN 0x01000000  // Extend change to start of growsdown vma (mprotect only).
#define PROT_GROWSUP   0x02000000  // Extend change to start of growsup vma (mprotect only).

/* Sharing types (must choose one and only one of these).  */
#define MAP_SHARED 0x01        // Share changes.
#define MAP_PRIVATE    0x02    // Changes are private.
# define MAP_SHARED_VALIDATE   0x03    // Share changes and validate extension flags.
# define MAP_TYPE  0x0f        // Mask for type of mapping.

char *allocate_share_page();

int ioctl_u(int fd, uint64 request, void *data);
void *mmap_u(void *addr, uint64 length, int prot, int flags, int fd, int64 offset);
int munmap_u(void *addr, uint64 length);
int read_mmap_u(char *dstva, char *src, uint64 count);

#endif
