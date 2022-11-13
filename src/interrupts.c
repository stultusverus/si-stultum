#include "interrupts.h"
#include "conlib.h"
#include "port_io.h"

__attribute__((interrupt)) void
PageFault_Handler(struct interrupt_frame *frame) {
  ERR("Page fault detected.", "\n", "Aborting...");
  putln("Aborted.");
  for (;;)
    ;
}

__attribute__((interrupt)) void
DoubleFault_Handler(struct interrupt_frame *frame) {
  ERR("Double fault detected.", "\n", "Aborting...");
  putln("Aborted.");
  for (;;)
    ;
}

__attribute__((interrupt)) void
GeneralProtectionFault_Handler(struct interrupt_frame *frame) {
  ERR("General protection fault detected.", "\n", "Aborting...");
  putln("Aborted.");
  for (;;)
    ;
}

__attribute__((interrupt)) void
Keyboard_Handler(struct interrupt_frame *frame) {
  puts(" KB");
  uint8_t scancode = inb(0x60);
  PIC_sendEOI(0);
}