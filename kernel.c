#include "kernel.h"

// Must be static. why?
static inline void plotpixel32(FrameBuffer *fb, int x, int y,
                               unsigned int pixel) {
  *((unsigned int *)(fb->fbbase + 4 * fb->ppsl * y + 4 * x)) = pixel;
}

void _start(FrameBuffer *fb) {
  for (int x = 0; x < fb->width; x++) {
    for (int y = 0; y < fb->height; y++) {
      plotpixel32(fb, x, y, 0x002fa7);
    }
  }
}