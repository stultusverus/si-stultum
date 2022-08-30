#ifndef __KERMEM_H__
#define __KERMEM_H__

#include "efimem.h"
#include <stdint.h>

typedef struct {
  uint64_t _size;
  uint32_t *_data;
} Bitmap;

void ppa_init_bitmap(uint32_t *data_addr, uint64_t size);
uint8_t ppa_bitmap_set(uint64_t index, uint8_t set_bit);
uint8_t ppa_bitmap_get(uint64_t index);

uint64_t get_memory_size(EfiMemoryDescriptor *mmap, uint64_t mmap_size,
                         uint64_t mmap_desc_size);

#endif
