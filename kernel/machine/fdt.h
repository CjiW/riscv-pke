// See LICENSE for license details.

#ifndef FDT_H
#define FDT_H
#include "util/types.h"
#include "kernel/riscv.h"
typedef struct {
  volatile uint32* ipi;
  volatile int mipi_pending;

  volatile uint64* timecmp;

  volatile uint32* plic_m_thresh;
  volatile uint32* plic_m_ie;
  volatile uint32* plic_s_thresh;
  volatile uint32* plic_s_ie;
} hls_t;
#define STACK_POINTER() ({            \
  uintptr_t __sp;                     \
  __asm__("mv %0, sp" : "=r"(__sp));  \
  __sp;                               \
})
#define RISCV_PGSHIFT 12
#define RISCV_PGSIZE (1 << RISCV_PGSHIFT)
#define HLS_SIZE 64
#define MACHINE_STACK_TOP()           \
  ({ (void*)((STACK_POINTER() + RISCV_PGSIZE) & -RISCV_PGSIZE); })

// hart-local storage, at top of stack
#define HLS() ((hls_t*)(MACHINE_STACK_TOP() - HLS_SIZE))
#define OTHER_HLS(id) ((hls_t*)((void*)HLS() + RISCV_PGSIZE * ((id) - read_const_csr(mhartid))))
#define FDT_MAGIC	0xd00dfeed
#define FDT_VERSION	17

#define IRQ_U_SOFT   0
#define IRQ_S_SOFT   1
#define IRQ_VS_SOFT  2
#define IRQ_M_SOFT   3
#define IRQ_U_TIMER  4
#define IRQ_S_TIMER  5
#define IRQ_VS_TIMER 6
#define IRQ_M_TIMER  7
#define IRQ_U_EXT    8
#define IRQ_S_EXT    9
#define IRQ_VS_EXT   10
#define IRQ_M_EXT    11
#define IRQ_S_GEXT   12
#define IRQ_COP      12
#define IRQ_HOST     13


#define FDT_BEGIN_NODE	1
#define FDT_END_NODE	2
#define FDT_PROP	3
#define FDT_NOP		4
#define FDT_END		9




// Setup memory+clint+plic
void query_plic(size_t fdt);
void hart_plic_init();



// Optional FDT preloaded external payload
extern void* kernel_start;
extern void* kernel_end;

#ifdef PK_PRINT_DEVICE_TREE
// Prints the device tree to the console as a DTS
void fdt_print(size_t fdt);
#endif

#endif
