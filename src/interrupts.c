#include "interrupts.h"
#include "chrono.h"
#include "conlib.h"
#include "logging.h"
#include "port_io.h"

__attribute__((interrupt)) void
PageFault_Handler(struct interrupt_frame *frame) {
  ERR("Page fault detected.", "\n", "Aborting...");
  kputs("Aborted.\n");
  for (;;)
    ;
}

__attribute__((interrupt)) void
DoubleFault_Handler(struct interrupt_frame *frame) {
  ERR("Double fault detected.", "\n", "Aborting...");
  kputs("Aborted.\n");
  for (;;)
    ;
}

__attribute__((interrupt)) void
GeneralProtectionFault_Handler(struct interrupt_frame *frame) {
  ERR("General protection fault detected.", "\n", "Aborting...");
  kputs("Aborted.\n");
  for (;;)
    ;
}

__attribute__((interrupt)) void
Keyboard_Handler(struct interrupt_frame *frame) {
  kputs(" KB\n");
  uint8_t scancode = inb(0x60);
  PIC_sendEOI(0);
}

__attribute__((interrupt)) void Timer_Handler(struct interrupt_frame *frame) {
  ms_count++;
  if (ms_count % 1000 == 0)
    kputs("hi. \n");
  PIC_sendEOI(0);
}