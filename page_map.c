#include "page_map.h"
#include "page_frames.h"
#include <stdint.h>

PageTable *PML4;

PageDirectoryEntry *pmm_flag_set(PageDirectoryEntry *entry, PdeBitFlag bit_mask,
                                 uint8_t val) {
  if (val)
    *entry |= bit_mask;
  else
    *entry &= ~bit_mask;
  return entry;
}

uint8_t pmm_flag_get(PageDirectoryEntry *entry, PdeBitFlag bit_mask) {
  PdeBitFlag it = 1;
  while (it >= bit_mask) {
    if (it & bit_mask && !(it & *entry))
      return 0;
    it <<= 1;
  }
  return 1;
}

PageDirectoryEntry *pmm_addr_set(PageDirectoryEntry *entry, uint64_t addr) {
  addr &= 0x000000ffffffffff;
  *entry &= 0xfff0000000000fff;
  *entry |= (addr << 12);
  return entry;
}

uint64_t pmm_addr_get(PageDirectoryEntry *entry) {
  return ((*entry) & 0x000ffffffffff000) >> 12;
}

PageTable *pmm_init_pml4(PageTable *addr) {
  PML4 = addr;
  return PML4;
}

void pmm_map_memory(void *vaddr, void *paddr) {
  PageDirectoryEntry pde;
  PageTable *pdp, *pd, *pt;
  uint64_t virtual_addr = (uint64_t)vaddr;
  uint64_t index;

  index = ((uint64_t)vaddr >> 39) & 0x1ff;
  pde = PML4->entries[index];
  if (!pmm_flag_get(&pde, PDE_P)) {
    pdp = (PageTable *)ppa_request();
    ppa_memset(pdp, 0, 4096);
    pmm_addr_set(&pde, (uint64_t)pdp >> 12);
    pmm_flag_set(&pde, PDE_P | PDE_RW, 1);
    PML4->entries[index] = pde;
  } else {
    pdp = (PageTable *)((uint64_t)pmm_addr_get(&pde) << 12);
  }

  index = ((uint64_t)vaddr >> 30) & 0x1ff;
  pde = pdp->entries[index];
  if (!pmm_flag_get(&pde, PDE_P)) {
    pd = (PageTable *)ppa_request();
    ppa_memset(pd, 0, 4096);
    pmm_addr_set(&pde, (uint64_t)pd >> 12);
    pmm_flag_set(&pde, PDE_P | PDE_RW, 1);
    pdp->entries[index] = pde;
  } else {
    pd = (PageTable *)((uint64_t)pmm_addr_get(&pde) << 12);
  }

  index = ((uint64_t)vaddr >> 21) & 0x1ff;
  pde = pd->entries[index];
  if (!pmm_flag_get(&pde, PDE_P)) {
    pt = (PageTable *)ppa_request();
    ppa_memset(pt, 0, 4096);
    pmm_addr_set(&pde, (uint64_t)pt >> 12);
    pmm_flag_set(&pde, PDE_P | PDE_RW, 1);
    pd->entries[index] = pde;
  } else {
    pt = (PageTable *)((uint64_t)pmm_addr_get(&pde) << 12);
  }

  index = ((uint64_t)vaddr >> 12) & 0x1ff;
  pde = pt->entries[index];
  pmm_addr_set(&pde, (uint64_t)paddr >> 12);
  pmm_flag_set(&pde, PDE_P | PDE_RW, 1);
  pt->entries[index] = pde;
}

void *pmm_find_paddr(void *vaddr) {
  PageDirectoryEntry pde;
  PageTable *pdp, *pd, *pt;
  uint64_t virtual_addr = (uint64_t)vaddr;
  uint64_t index;

  index = ((uint64_t)vaddr >> 39) & 0x1ff;
  pde = PML4->entries[index];
  if (!pmm_flag_get(&pde, PDE_P)) {
    return 0;
  } else {
    pdp = (PageTable *)((uint64_t)pmm_addr_get(&pde) << 12);
  }

  index = ((uint64_t)vaddr >> 30) & 0x1ff;
  pde = pdp->entries[index];
  if (!pmm_flag_get(&pde, PDE_P)) {
    return 0;
  } else {
    pd = (PageTable *)((uint64_t)pmm_addr_get(&pde) << 12);
  }

  index = ((uint64_t)vaddr >> 21) & 0x1ff;
  pde = pd->entries[index];
  if (!pmm_flag_get(&pde, PDE_P)) {
    return 0;
  } else {
    pt = (PageTable *)((uint64_t)pmm_addr_get(&pde) << 12);
  }

  index = ((uint64_t)vaddr >> 12) & 0x1ff;
  pde = pt->entries[index];
  if (!pmm_flag_get(&pde, PDE_P)) {
    return 0;
  }
  return (void *)(pmm_addr_get(&pde) << 12);
}