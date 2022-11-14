#include "page_frames.h"
#include "efimem.h"

Bitmap ppa_bitmap = {
    ._size      = 0,
    ._max_index = 0,
    ._data      = 0,
};

uint64_t mem_used;
uint64_t mem_rsvd;
uint64_t mem_free;
uint64_t mem_total;

uint8_t ppa_initialized = 0;
uint64_t last_hit       = 0;

static inline void ppa_init_bitmap(uint32_t *data_addr, uint64_t pages) {
  uint64_t size         = pages / 32 + 1;
  ppa_bitmap._size      = size;
  ppa_bitmap._max_index = pages - 1;
  ppa_bitmap._data      = data_addr;
  for (int i = 0; i < size; i++)
    ppa_bitmap._data[i] = 0;
}

uint64_t ppa_get_mem_used() { return mem_used; }
uint64_t ppa_get_mem_rsvd() { return mem_rsvd; }
uint64_t ppa_get_mem_free() { return mem_free; }
uint64_t ppa_get_mem_total() { return mem_total; }

uint64_t ppa_get_bitmap_size() { return ppa_bitmap._size; }

void ppa_init(EfiMemoryDescriptor *mmap, uint64_t mmap_size,
              uint64_t mmap_desc_size) {
  if (ppa_initialized)
    return;
  mem_used              = 0;
  mem_rsvd              = 0;
  mem_free              = 0;
  mem_total             = 0;
  void *largest_segment = 0;
  uint64_t largest_size = 0;
  for (EfiMemoryDescriptor *desc = mmap;
       (uint8_t *)desc < (uint8_t *)mmap + mmap_size;
       desc = ((EfiMemoryDescriptor *)(((uint8_t *)desc) + mmap_desc_size))) {
    uint64_t crt_size = desc->numberofpages * 4096;
    mem_total += crt_size;
    if (desc->type == 7 && crt_size > largest_size) {
      largest_segment = (void *)desc->physicalstart;
      largest_size    = crt_size;
    }
  }
  mem_free = mem_total;

  if (largest_segment == 0)
    return;

  uint64_t total_pages = mem_total / 4096;
  ppa_init_bitmap(largest_segment, total_pages);

  uint64_t bitmap_pages =
      ppa_bitmap._size / 1024 + (ppa_bitmap._size % 1024 ? 1 : 0);
  ppa_rsvn(0, total_pages);

  for (EfiMemoryDescriptor *desc = mmap;
       (uint8_t *)desc < (uint8_t *)mmap + mmap_size;
       desc = ((EfiMemoryDescriptor *)(((uint8_t *)desc) + mmap_desc_size))) {
    if (desc->type == 7) {
      ppa_ursvn((void *)desc->physicalstart, desc->numberofpages);
    }
  }

  ppa_lckn(ppa_bitmap._data, bitmap_pages);
  ppa_lck(&ppa_bitmap);
  ppa_initialized = 1;
}

uint8_t ppa_bitmap_set(uint64_t index, uint8_t set_bit) {
  if (index > ppa_bitmap._max_index)
    return 0;
  uint32_t mask = 1 << (index % 32);
  if (set_bit)
    ppa_bitmap._data[index / 32] |= mask;
  else
    ppa_bitmap._data[index / 32] &= ~mask;
  return ppa_bitmap._data[index / 32] & mask ? 1 : 0;
}

uint8_t ppa_bitmap_get(uint64_t index) {
  if (index > ppa_bitmap._max_index)
    return 0;
  uint32_t mask = 1 << (index % 32);
  return ppa_bitmap._data[index / 32] & mask ? 1 : 0;
}

uint64_t get_memory_size(EfiMemoryDescriptor *mmap, uint64_t mmap_size,
                         uint64_t mmap_desc_size) {
  uint64_t ret = 0;
  for (EfiMemoryDescriptor *desc = mmap;
       (uint8_t *)desc < (uint8_t *)mmap + mmap_size;
       desc = ((EfiMemoryDescriptor *)(((uint8_t *)desc) + mmap_desc_size))) {
    ret += desc->numberofpages * 4096;
  }
  return ret;
}

void ppa_lck(void *addr) {
  uint64_t index = (uint64_t)addr / 4096;
  if (ppa_bitmap_get(index))
    return;
  ppa_bitmap_set(index, 1);
  mem_used += 4096;
  mem_free -= 4096;
}

void ppa_lckn(void *addr, uint64_t n) {
  while (n--) {
    ppa_lck(addr);
    addr = (void *)((uint64_t)addr + 4096);
  }
}

void ppa_ulck(void *addr) {
  uint64_t index = (uint64_t)addr / 4096;
  if (!ppa_bitmap_get(index))
    return;
  ppa_bitmap_set(index, 0);
  mem_used -= 4096;
  mem_free += 4096;
  last_hit = index;
}

void ppa_ulckn(void *addr, uint64_t n) {
  for (int i = 0; i < n; i++) {
    ppa_ulck((void *)((uint64_t)addr + 4096 * i));
  }
  last_hit = (uint64_t)addr / 4096;
}

void ppa_rsv(void *addr) {
  uint64_t index = (uint64_t)addr / 4096;
  if (ppa_bitmap_get(index))
    return;
  ppa_bitmap_set(index, 1);
  mem_rsvd += 4096;
  mem_free -= 4096;
}

void ppa_rsvn(void *addr, uint64_t n) {
  while (n--) {
    ppa_rsv(addr);
    addr = (void *)((uint64_t)addr + 4096);
  }
}

void ppa_ursv(void *addr) {
  uint64_t index = (uint64_t)addr / 4096;
  if (!ppa_bitmap_get(index))
    return;
  ppa_bitmap_set(index, 0);
  mem_rsvd -= 4096;
  mem_free += 4096;
  last_hit = index;
}

void ppa_ursvn(void *addr, uint64_t n) {
  for (int i = 0; i < n; i++) {
    ppa_ursv((void *)((uint64_t)addr + 4096 * i));
  }
  last_hit = (uint64_t)addr / 4096;
}

void *ppa_request() {
  for (uint64_t index_hi = last_hit >> 5; index_hi < ppa_bitmap._size;
       index_hi++) {
    if (!~ppa_bitmap._data[index_hi])
      continue;
    for (uint64_t index_lo = 0; index_lo < 32; index_lo++) {
      uint64_t index = index_hi * 32 + index_lo;
      if (ppa_bitmap_get(index))
        continue;
      last_hit = index;
      ppa_lck((void *)(index * 4096));
      return (void *)(index * 4096);
    }
  }
  return 0;
}

void *ppa_memset(void *dest, uint8_t val, uint64_t len) {
  register uint8_t *ptr = (uint8_t *)dest;
  while (len-- > 0)
    *ptr++ = val;
  return dest;
}

void *ppa_memcpy(void *dest, const void *src, uint64_t len) {
  uint8_t *d       = dest;
  uint8_t const *s = src;
  while (len--)
    *d++ = *s++;
  return dest;
}