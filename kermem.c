#include "kermem.h"

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