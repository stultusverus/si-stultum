#ifndef __EFIMEM_H__
#define __EFIMEM_H__

#include <stdint.h>

typedef struct {
  uint32_t type; // field size is 32 bits followed by 32 bit pad
  uint32_t pad;
  uint64_t physicalstart; // field size is 64 bits
  uint64_t virtualstart;  // field size is 64 bits
  uint64_t numberofpages; // field size is 64 bits
  uint64_t attribute;     // field size is 64 bits
} EfiMemoryDescriptor;

extern const char *EFI_MEMORY_TYPE_STRINGS[];

#endif // __EFIMEM_H__