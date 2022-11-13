#include "port_io.h"

#define PIC_EOI 0x20 /* End-of-interrupt command code */

void PIC_sendEOI(unsigned char irq) {
  if (irq >= 8)
    outb(PIC2_COMMAND, PIC_EOI);
  outb(PIC1_COMMAND, PIC_EOI);
}

void PIC_remap(int offset1, int offset2) {
  unsigned char a1, a2;

  a1 = inb(PIC1_DATA); // save masks
  a2 = inb(PIC2_DATA);

  // starts the initialization sequence (in cascade mode)
  outb(PIC1_COMMAND, ICW1_INIT | ICW1_ICW4);
  io_wait();
  outb(PIC2_COMMAND, ICW1_INIT | ICW1_ICW4);
  io_wait();

  outb(PIC1_DATA, offset1); // ICW2: Master PIC vector offset
  io_wait();
  outb(PIC2_DATA, offset2); // ICW2: Slave PIC vector offset
  io_wait();

  // ICW3: tell Master PIC that there is a slave PIC at IRQ2 (0000 0100)
  outb(PIC1_DATA, 4);
  io_wait();
  // ICW3: tell Slave PIC its cascade identity (0000 0010)
  outb(PIC2_DATA, 2);
  io_wait();

  outb(PIC1_DATA, ICW4_8086);
  io_wait();
  outb(PIC2_DATA, ICW4_8086);
  io_wait();

  outb(PIC1_DATA, a1); // restore saved masks.
  outb(PIC2_DATA, a2);
}
