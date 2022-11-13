#ifndef __PORT_IO_H__
#define __PORT_IO_H__

#include <stdint.h>

#define PIC1            0x20 /* IO base address for master PIC */
#define PIC2            0xA0 /* IO base address for slave PIC */
#define PIC1_COMMAND    PIC1
#define PIC1_DATA       (PIC1 + 1)
#define PIC2_COMMAND    PIC2
#define PIC2_DATA       (PIC2 + 1)

#define ICW1_ICW4       0x01 /* ICW4 (not) needed */
#define ICW1_SINGLE     0x02 /* Single (cascade) mode */
#define ICW1_INTERVAL4  0x04 /* Call address interval 4 (8) */
#define ICW1_LEVEL      0x08 /* Level triggered (edge) mode */
#define ICW1_INIT       0x10 /* Initialization - required! */

#define ICW4_8086       0x01 /* 8086/88 (MCS-80/85) mode */
#define ICW4_AUTO       0x02 /* Auto (normal) EOI */
#define ICW4_BUF_SLAVE  0x08 /* Buffered mode/slave */
#define ICW4_BUF_MASTER 0x0C /* Buffered mode/master */
#define ICW4_SFNM       0x10 /* Special fully nested (not) */

static inline void outb(uint16_t port, uint8_t val) {
  asm volatile("outb %0, %1" : : "a"(val), "Nd"(port));
  /* There's an outb %al, $imm8  encoding, for compile-time constant port
   * numbers that fit in 8b.  (N constraint). Wider immediate constants would be
   * truncated at assemble-time (e.g. "i" constraint). The  outb  %al, %dx
   * encoding is the only option for all other cases. %1 expands to %dx because
   * port  is a uint16_t.  %w1 could be used if we had the port number a wider C
   * type */
}

static inline uint8_t inb(uint16_t port) {
  uint8_t ret;
  asm volatile("inb %1, %0" : "=a"(ret) : "Nd"(port));
  return ret;
}

static inline void io_wait(void) { outb(0x80, 0); }

static inline void enable() { asm("sti"); }

void PIC_remap(int offset1, int offset2);
void PIC_sendEOI(unsigned char irq);

#endif // __PORT_IO_H__