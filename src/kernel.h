#ifndef __KERNEL_H__
#define __KERNEL_H__

#include "efimem.h"
#include <stdint.h>

typedef struct {
  uint64_t fbbase;
  uint64_t fbsize;
  uint32_t width;
  uint32_t height;
  uint32_t ppsl;
} FrameBuffer;

typedef struct {
  FrameBuffer *fb;
  EfiMemoryDescriptor *mmap;
  uint64_t mmap_size;
  uint64_t mmap_desc_size;
} BootInfo;

#endif // __KERNEL_H__