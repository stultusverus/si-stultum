#include "idt.h"

IDTEntry idt_create_entry(uint64_t offset, uint16_t selector, uint8_t ist,
                          IDT_GATE_TA type_attributes) {
  IDTEntry ret;
  ret.offset_0 = offset & 0xffff;
  ret.offset_1 = (offset >> 16) & 0xffff;
  ret.offset_2 = (offset >> 32) & 0xffffffff;
  ret.selector = selector;
  ret.ist = ist;
  if (type_attributes == IDT_INTERRUPT_GATE || type_attributes == IDT_TRAP_GATE)
    ret.type_attributes = type_attributes;
  else
    ret.type_attributes = 0x00;
  ret.zero = 0x00000000;
  return ret;
}
