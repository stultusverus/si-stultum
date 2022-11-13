#ifndef __PAGE_MAP_H__
#define __PAGE_MAP_H__
#include <stdint.h>

typedef uint64_t PageDirectoryEntry;

typedef enum {
  PDE_P = 1 << 0,
  PDE_RW = 1 << 1,
  PDE_US = 1 << 2,
  PDE_PWT = 1 << 3,
  PDE_PCD = 1 << 4,
  PDE_A = 1 << 5,
  PDE_AVL = 1 << 6,
  PDE_PS = 1 << 7,
} PdeBitFlag;

typedef struct {
  PageDirectoryEntry entries[512];
} __attribute__((aligned(0x1000))) PageTable;

// extern PageTable *PML4;

PageDirectoryEntry *pmm_flag_set(PageDirectoryEntry *entry, PdeBitFlag bit_mask,
                                 uint8_t val);
uint8_t pmm_flag_get(PageDirectoryEntry *entry, PdeBitFlag bit_mask);

PageDirectoryEntry *pmm_addr_set(PageDirectoryEntry *entry, uint64_t addr);
uint64_t pmm_addr_get(PageDirectoryEntry *entry);

PageTable *pmm_init_pml4(PageTable *addr);
void pmm_map_memory(void *vaddr, void *paddr);
void *pmm_find_paddr(void *vaddr);

void set_pml4(PageTable *);

#endif // __PAGE_MAP_H__
