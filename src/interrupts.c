#include "interrupts.h"
#include "conlib.h"

__attribute__((interrupt)) void
PageFault_Handler(struct interrupt_frame *frame) {
  putln("Page fault detected");
  for (;;)
    ;
}