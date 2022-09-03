#ifndef __GDT_H__
#define __GDT_H__

#include <stdint.h>

typedef struct {
  uint16_t size;
  uint64_t offset;
} __attribute__((packed)) GDTDescriptor;

typedef struct {
  uint16_t limit0;
  uint16_t base0;
  uint8_t base1;
  uint8_t accessbyte;
  uint8_t limit1_flags;
  uint8_t base2;
} __attribute__((packed)) GDTEntry;

typedef struct {
  GDTEntry null0;
  GDTEntry kernel_code;
  GDTEntry kernel_data;
  GDTEntry null1;
  GDTEntry user_code;
  GDTEntry user_data;
} __attribute__((packed)) __attribute__((aligned(0x1000))) GDT;

extern GDT DEFAULT_GDT;

void set_gdt(GDTDescriptor *);

#endif