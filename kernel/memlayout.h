#ifndef _MEMLAYOUT_H
#define _MEMLAYOUT_H
#include "riscv.h"

// RISC-V machine places its physical memory above DRAM_BASE
#define DRAM_BASE 0x80000000

// the beginning virtual address of PKE kernel
#define KERN_BASE 0x80000000

// default stack size
#define STACK_SIZE 4096

// virtual address of stack top of user process
#define USER_STACK_TOP 0x7ffff000

// start virtual address (4MB) of our simple heap. added @lab2_2
#define USER_FREE_ADDRESS_START 0x00000000 + PGSIZE * 1024

// start virtual address (8MB) of our shared memory. added @lab5_3
#define USER_SHARE_MEMORY_START 0x00000000 + PGSIZE * 1024 * 2

// start virtual address (12MB) of our mmap memory. added @lab5_3
#define USER_MMAP_MEMORY_START 0x00000000 + PGSIZE * 1024 * 3

#endif
