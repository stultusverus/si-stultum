#include "chrono.h"

#include "port_io.h"

// qemu under wsl has not kvm support.
// implement this later.

volatile uint64_t ms_count;

void init_PIT(uint32_t frequency) {
  uint32_t divisor = 1193180 / frequency;
  ms_count         = 0;

  outb(0x43, 0x36); // command mode channel

  uint8_t lo = (uint8_t)(divisor & 0xFF);
  uint8_t hi = (uint8_t)((divisor >> 8) & 0xFF);

  clear_interrupts();

  outb(0x40, lo); // data channel 0
  outb(0x40, hi);

  // IRQ_clear_mask(0);
}