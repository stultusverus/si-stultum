#ifndef __KERNEL_H__
#define __KERNEL_H__

#include "efimem.h"
#include <stdint.h>

const uint32_t CPUID_FLAG_MSR = 1 << 5;

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

static inline void cpuid(int code, uint32_t *a, uint32_t *d) {
  asm volatile("cpuid" : "=a"(*a), "=d"(*d) : "a"(code) : "ecx", "ebx");
}

static inline int cpuid_string(int code, uint32_t where[4]) {
  asm volatile("cpuid"
               : "=a"(*where), "=b"(*(where + 1)), "=c"(*(where + 2)),
                 "=d"(*(where + 3))
               : "a"(code));
  return (int)where[0];
}

static inline int cpu_has_msr() {
  uint32_t a, d;
  cpuid(1, &a, &d);
  return d & CPUID_FLAG_MSR;
}

static inline void cpu_get_msr(uint32_t msr, uint32_t *lo, uint32_t *hi) {
  asm volatile("rdmsr" : "=a"(*lo), "=d"(*hi) : "c"(msr));
}

static inline void cpu_set_msr(uint32_t msr, uint32_t lo, uint32_t hi) {
  asm volatile("wrmsr" : : "a"(lo), "d"(hi), "c"(msr));
}

#endif // __KERNEL_H__