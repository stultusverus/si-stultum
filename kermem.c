#include "kermem.h"
#include "efimem.h"

Bitmap ppa_bitmap = {
    ._data = 0,
    ._size = 0,
};

uint64_t mem_used;
uint64_t mem_reserved;
uint64_t mem_free;
uint64_t mem_total;
uint8_t ppa_initialized = 0;

void ppa_init(EfiMemoryDescriptor *mmap, uint64_t mmap_size,
              uint64_t mmap_desc_size) {
  if (ppa_initialized)
    return;
  mem_used = 0;
  mem_reserved = 0;
  mem_free = 0;
  mem_total = 0;
  void *largest_segment = 0;
  uint64_t largest_size = 0;
  for (EfiMemoryDescriptor *desc = mmap;
       (uint8_t *)desc < (uint8_t *)mmap + mmap_size;
       desc = ((EfiMemoryDescriptor *)(((uint8_t *)desc) + mmap_desc_size))) {
    uint64_t crt_size = desc->numberofpages * 4096;
    mem_total += crt_size;
    if (desc->type == 7) {
      mem_free += crt_size;
      if (crt_size > largest_size) {
        largest_segment = (void *)desc->physicalstart;
        largest_size = crt_size;
      }
    } else {
      mem_reserved += crt_size;
    }
  }

  if (largest_segment == 0) // no memory? impossible.
    return;

  uint64_t bitmap_data_size = mem_total / 4096 / 32 + 1;
  ppa_init_bitmap(largest_segment, bitmap_data_size);

  // TODO: lock pages of bitmap
  // TODO: reserve unusable pages

  ppa_initialized = 1;
}

void ppa_init_bitmap(uint32_t *data_addr, uint64_t size) {
  ppa_bitmap._data = data_addr;
  ppa_bitmap._size = size;
  for (int i = 0; i < size / 4096 / 32 + 1; i++)
    ppa_bitmap._data[i] = 0;
}

uint8_t ppa_bitmap_set(uint64_t index, uint8_t set_bit) {
  if (index > ppa_bitmap._size)
    return 0;
  uint32_t mask = 1 << (index % 32);
  if (set_bit)
    ppa_bitmap._data[index / 32] |= mask;
  else
    ppa_bitmap._data[index / 32] &= ~mask;
  return ppa_bitmap._data[index / 32] & mask ? 1 : 0;
}

uint8_t ppa_bitmap_get(uint64_t index) {
  if (index > ppa_bitmap._size)
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
