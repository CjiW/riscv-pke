#include "fdt.h"
#include "spike_interface/spike_utils.h"
#include "util/string.h"
//#include "mtrap.h"
#include "spike_interface/dts_parse.h"
#include "util/types.h"

static inline uint32 bswap(uint32 x)
{
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
  uint32 y = (x & 0x00FF00FF) <<  8 | (x & 0xFF00FF00) >>  8;
  uint32 z = (y & 0x0000FFFF) << 16 | (y & 0xFFFF0000) >> 16;
  return z;
#else
  /* No need to swap on big endian */
  return x;
#endif
}

static inline int isstring(char c)
{
  if (c >= 'A' && c <= 'Z')
    return 1;
  if (c >= 'a' && c <= 'z')
    return 1;
  if (c >= '0' && c <= '9')
    return 1;
  if (c == '\0' || c == ' ' || c == ',' || c == '-')
    return 1;
  return 0;
}

static uint32 *fdt_scan_helper(
  uint32 *lex,
  const char *strings,
  struct fdt_scan_node *node,
  const struct fdt_cb *cb)
{
  struct fdt_scan_node child;
  struct fdt_scan_prop prop;
  int last = 0;

  child.parent = node;
  // these are the default cell counts, as per the FDT spec
  child.address_cells = 2;
  child.size_cells = 1;
  prop.node = node;

  while (1) {
    switch (bswap(lex[0])) {
      case FDT_NOP: {
        lex += 1;
        break;
      }
      case FDT_PROP: {
        assert (!last);
        prop.name  = strings + bswap(lex[2]);
        prop.len   = bswap(lex[1]);
        prop.value = lex + 3;
        if (node && !strcmp(prop.name, "#address-cells")) { node->address_cells = bswap(lex[3]); }
        if (node && !strcmp(prop.name, "#size-cells"))    { node->size_cells    = bswap(lex[3]); }
        lex += 3 + (prop.len+3)/4;
        cb->prop(&prop, cb->extra);
        break;
      }
      case FDT_BEGIN_NODE: {
        uint32 *lex_next;
        if (!last && node && cb->done) cb->done(node, cb->extra);
        last = 1;
        child.name = (const char *)(lex+1);
        if (cb->open) cb->open(&child, cb->extra);
        lex_next = fdt_scan_helper(
          lex + 2 + strlen(child.name)/4,
          strings, &child, cb);
        if (cb->close && cb->close(&child, cb->extra) == -1)
          while (lex != lex_next) *lex++ = bswap(FDT_NOP);
        lex = lex_next;
        break;
      }
      case FDT_END_NODE: {
        if (!last && node && cb->done) cb->done(node, cb->extra);
        return lex + 1;
      }
      default: { // FDT_END
        if (!last && node && cb->done) cb->done(node, cb->extra);
        return lex;
      }
    }
  }
}


struct plic_scan
{
  int compat;
  uint64 reg;
  uint32 *int_value;
  int int_len;
  int done;
  int ndev;
};
static void plic_open(const struct fdt_scan_node *node, void *extra)
{
  struct plic_scan *scan = (struct plic_scan *)extra;
  scan->compat = 0;
  scan->reg = 0;
  scan->int_value = 0;
}

static void plic_prop(const struct fdt_scan_prop *prop, void *extra)
{
  struct plic_scan *scan = (struct plic_scan *)extra;
  if (!strcmp(prop->name, "compatible") && fdt_string_list_index(prop, "riscv,plic0") >= 0) {
    scan->compat = 1;
  } else if (!strcmp(prop->name, "reg")) {
    fdt_get_address(prop->node->parent, prop->value, &scan->reg);
  } else if (!strcmp(prop->name, "interrupts-extended")) {
    scan->int_value = prop->value;
    scan->int_len = prop->len;
  } else if (!strcmp(prop->name, "riscv,ndev")) {
    scan->ndev = bswap(prop->value[0]);
  }
}
#define HART_BASE	0x200000
#define HART_SIZE	0x1000
#define ENABLE_BASE	0x2000
#define ENABLE_SIZE	0x80
extern volatile uint32* plic_priorities;
extern ssize_t plic_ndevs;
static void plic_done(const struct fdt_scan_node *node, void *extra)
{
  struct plic_scan *scan = (struct plic_scan *)extra;
  const uint32 *value = scan->int_value;
  const uint32 *end = value + scan->int_len/4;

  if (!scan->compat) return;
  assert (scan->reg != 0);
  assert (scan->int_value && scan->int_len % 8 == 0);
  assert (scan->ndev >= 0 && scan->ndev < 1024);
  assert (!scan->done); // only one plic

  scan->done = 1;
  plic_priorities = (uint32*)(uintptr_t)scan->reg;
  plic_ndevs = scan->ndev;

  for (int index = 0; end - value > 0; ++index) {
    uint32 phandle = bswap(value[0]);
    uint32 cpu_int = bswap(value[1]);
    int hart = 0;

      hls_t *hls = OTHER_HLS(hart);
      if (cpu_int == IRQ_M_EXT) {
        hls->plic_m_ie     = (uint32*)((uintptr_t)scan->reg + ENABLE_BASE + ENABLE_SIZE * index);
        hls->plic_m_thresh = (uint32*) ((uintptr_t)scan->reg + HART_BASE   + HART_SIZE   * index);
      } else if (cpu_int == IRQ_S_EXT) {
        hls->plic_s_ie     = (uint32*)((uintptr_t)scan->reg + ENABLE_BASE + ENABLE_SIZE * index);
        hls->plic_s_thresh = (uint32*) ((uintptr_t)scan->reg + HART_BASE   + HART_SIZE   * index);
      } else {
        sprint("PLIC wired hart %d to wrong interrupt %d", hart, cpu_int);
      }

    value += 2;
  }
}
void query_plic(uintptr_t fdt)
{
  struct fdt_cb cb;
  struct plic_scan scan;

  memset(&cb, 0, sizeof(cb));
  cb.open = plic_open;
  cb.prop = plic_prop;
  cb.done = plic_done;
  cb.extra = &scan;

  scan.done = 0;
  fdt_scan(fdt, &cb);
}
void hart_plic_init()
{
  // clear pending interrupts
  // *HLS()->ipi = 0;
  // *HLS()->timecmp = -1ULL;
  write_csr(mip, 0);

  if (!plic_ndevs)
    return;

  size_t ie_words = (plic_ndevs + 8 * sizeof(*HLS()->plic_s_ie) - 1) /
    (8 * sizeof(*HLS()->plic_s_ie));
  for (size_t i = 0; i < ie_words; i++) {
     if (HLS()->plic_s_ie) {
        // Supervisor not always present
        HLS()->plic_s_ie[i] = __UINT32_MAX__;
     }
     //////////////////////////////
     HLS()->plic_m_ie[i] = __UINT32_MAX__;
  }
  *HLS()->plic_m_thresh = 1;
  if (HLS()->plic_s_thresh) {
      // Supervisor not always present
      *HLS()->plic_s_thresh = 0;
  }
}