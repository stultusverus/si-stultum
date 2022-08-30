#include "kernel.h"
#include "conlib.h"

extern unsigned char _binary_u_vga16_sfn_start;

// Must be static. why?
static inline void plotpixel32(FrameBuffer *fb, int x, int y,
                               unsigned int pixel) {
  *((unsigned int *)(fb->fbbase + 4 * fb->ppsl * y + 4 * x)) = pixel;
}

void _start(FrameBuffer *fb) {
  char buff[100];
  cinit((ssfn_font_t *)&_binary_u_vga16_sfn_start, (void *)fb->fbbase,
        fb->width, fb->height, fb->ppsl * 4);
  citoaul(0x80000000, buff, 16);
  for (;;) {
    cputs("base addr: ");
    cputs(buff);
  }
}