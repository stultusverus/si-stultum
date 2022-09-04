#ifndef __INTERRUPTS_H__
#define __INTERRUPTS_H__

struct interrupt_frame;
__attribute__((interrupt)) void
PageFault_Handler(struct interrupt_frame *frame);

#endif // __INTERRUPTS_H__