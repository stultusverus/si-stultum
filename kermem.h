#ifndef __KERMEM_H__
#define __KERMEM_H__

#include "efimem.h"
#include <stdint.h>

uint64_t get_memory_size(EfiMemoryDescriptor *mmap, uint64_t mmap_size,
                         uint64_t mmap_desc_size);

#endif
