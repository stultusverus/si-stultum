#include "kernel.h"
#define SSFN_CONSOLEBITMAP_TRUECOLOR
// #define SSFN_CONSOLEBITMAP_CONTROL
#include "ssfn.h"

extern unsigned char _binary_u_vga16_sfn_start;

// Must be static. why?
static inline void plotpixel32(FrameBuffer *fb, int x, int y,
                               unsigned int pixel) {
  *((unsigned int *)(fb->fbbase + 4 * fb->ppsl * y + 4 * x)) = pixel;
}

void _start(FrameBuffer *fb) {
  /* set up context by global variables */
  ssfn_src =
      (ssfn_font_t *)&_binary_u_vga16_sfn_start; /* the bitmap font to use */
  ssfn_dst.ptr =
      (void *)fb->fbbase; /* framebuffer address and bytes per line */
  ssfn_dst.w = fb->width;
  ssfn_dst.h = fb->height;
  ssfn_dst.x = 0; /* coordinates to draw to */
  ssfn_dst.y = 0;
  ssfn_dst.p = fb->ppsl * 4;
  ssfn_dst.fg = 0xffffffff; /* colors, white on black */
  ssfn_dst.bg = 0;

  ssfn_putc('H');
  ssfn_putc('e');
  ssfn_putc('l');
  ssfn_putc('l');
  ssfn_putc('o');
}