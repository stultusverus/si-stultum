#ifndef __PAGE_FRAMES_H__
#define __PAGE_FRAMES_H__

#include "efimem.h"
#include <stdint.h>

typedef struct {
  uint64_t _size;
  uint64_t _max_index;
  uint32_t *_data;
} Bitmap;

// void ppa_init_bitmap(uint32_t *data_addr, uint64_t size);
void ppa_init(EfiMemoryDescriptor *mmap, uint64_t mmap_size,
              uint64_t mmap_desc_size);
uint8_t ppa_bitmap_set(uint64_t index, uint8_t set_bit);
uint8_t ppa_bitmap_get(uint64_t index);

uint64_t ppa_get_mem_used();
uint64_t ppa_get_mem_rsvd();
uint64_t ppa_get_mem_free();
uint64_t ppa_get_mem_total();
uint64_t ppa_get_bitmap_size();
// uint32_t ppa_get_bitmap_data_addr();

void ppa_lck(void *addr);
void ppa_lckn(void *addr, uint64_t n);
void ppa_ulck(void *addr);
void ppa_ulckn(void *addr, uint64_t n);
void ppa_rsv(void *addr);
void ppa_rsvn(void *addr, uint64_t n);
void ppa_ursv(void *addr);
void ppa_ursvn(void *addr, uint64_t n);
void *ppa_request();

void *ppa_memset(void *dest, uint8_t val, uint64_t len);
void *ppa_memcpy(void *dest, const void *src, uint64_t len);

uint64_t get_memory_size(EfiMemoryDescriptor *mmap, uint64_t mmap_size,
                         uint64_t mmap_desc_size);

#endif // __PAGE_FRAMES_H__
