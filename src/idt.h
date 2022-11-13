#ifndef __IDT_H__
#define __IDT_H__
#include <stdint.h>

typedef enum {
  IDT_INTERRUPT_GATE = 0x8E,
  IDT_TRAP_GATE = 0x8F,
} IDT_GATE_TA;

typedef struct {
  uint16_t size; // in bytes, always 4095
  uint64_t offset;
} __attribute__((packed)) IDTDescriptor;

typedef struct {
  uint16_t offset_0;       // offset bits 0..15
  uint16_t selector;       // a code segment selector in GDT or LDT
  uint8_t ist;             // bits 0..2 holds IST offset, rest of bits zero.
  uint8_t type_attributes; // gate type, dpl, and p fields
  uint16_t offset_1;       // offset bits 16..31
  uint32_t offset_2;       // offset bits 32..63
  uint32_t zero;           // reserved
} __attribute__((packed)) IDTEntry;

typedef IDTEntry IDT[256];

#define IDT_CREATE_ENTRY(OFFSET, SELECTOR, IST, TYPE_ATTR)                     \
  ((IDTEntry){                                                                 \
      .offset_0 = ((OFFSET)&0xffff),                                           \
      .offset_1 = ((OFFSET >> 16) & 0xffff),                                   \
      .offset_2 = ((OFFSET >> 32) & 0xffffffff),                               \
      .selector = (SELECTOR),                                                  \
      .ist = (IST),                                                            \
      .type_attributes =                                                       \
          ((TYPE_ATTR) == IDT_INTERRUPT_GATE || (TYPE_ATTR) == IDT_TRAP_GATE   \
               ? (TYPE_ATTR)                                                   \
               : 0x00),                                                        \
      .zero = 0x00000000,                                                      \
  })

IDTEntry idt_create_entry(uint64_t offset, uint16_t selector, uint8_t ist,
                          IDT_GATE_TA type_attributes);

void set_idt(IDTDescriptor *);

#endif //__IDT_H__
