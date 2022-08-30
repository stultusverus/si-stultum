#include "kernel.h"
#include "conlib.h"

extern unsigned char _binary_u_vga16_sfn_start;

// Must be static. why?
static inline void plotpixel32(FrameBuffer *fb, int x, int y,
                               unsigned int pixel) {
  *((unsigned int *)(fb->fbbase + 4 * fb->ppsl * y + 4 * x)) = pixel;
}

void _start(BootInfo boot_info) {
  char buff[100];
  FrameBuffer *fb = boot_info.fb;
  void *mmap = boot_info.mmap;
  cinit((ssfn_font_t *)&_binary_u_vga16_sfn_start, (void *)fb->fbbase,
        fb->width, fb->height, fb->ppsl * 4);
  cputs("Framebuffer Address: 0x");
  cputln(citoaul((unsigned long)fb->fbbase, buff, 16));
  cputs("Memory Map: 0x");
  cputln(citoaul((unsigned long)mmap, buff, 16));
  cputs("Memory Map Size: ");
  cputln(citoaul((unsigned long)boot_info.mmap_size, buff, 10));
  cputs("Memory Map Descripotr Size: ");
  cputln(citoaul((unsigned long)boot_info.mmap_desc_size, buff, 10));
}