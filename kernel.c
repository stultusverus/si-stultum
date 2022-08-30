#include "kernel.h"
#include "conlib.h"
#include "kermem.h"

extern uint8_t _binary_u_vga16_sfn_start;
extern uint8_t _kernel_start;
extern uint8_t _kernel_end;

// Must be static. why?
static inline void plotpixel32(FrameBuffer *fb, int x, int y,
                               unsigned int pixel) {
  *((unsigned int *)(fb->fbbase + 4 * fb->ppsl * y + 4 * x)) = pixel;
}

void _start(BootInfo boot_info) {
  char buff[100];
  uint64_t mmap_entries = boot_info.mmap_size / boot_info.mmap_desc_size;

  cinit((ssfn_font_t *)&_binary_u_vga16_sfn_start, (void *)boot_info.fb->fbbase,
        boot_info.fb->width, boot_info.fb->height, boot_info.fb->ppsl * 4);
  cputs("Framebuffer Address: 0x");
  cputln(citoaul((unsigned long)boot_info.fb->fbbase, buff, 16));
  cputs("Memory Map: @[0x");
  cputs(citoaul((unsigned long)boot_info.mmap, buff, 16));
  cputs("], ");
  cputs(citoaul(mmap_entries, buff, 10));
  cputs(" entries, desc_size=0x");
  cputs(citoaul(boot_info.mmap_desc_size, buff, 16));
  cputs(", total size is ");
  cputs(citoaul(get_memory_size(boot_info.mmap, boot_info.mmap_size,
                                boot_info.mmap_desc_size),
                buff, 10));
  cputln(" Bytes.");

  ppa_init(boot_info.mmap, boot_info.mmap_size, boot_info.mmap_desc_size);
  // uint64_t kernel_size = &_kernel_end - &_kernel_start;
  // ppa_lckn(&_kernel_start, kernel_size / 4096 + 1);

  cputs("Used: ");
  cputs(citoaul(ppa_get_mem_used() / 1024, buff, 10));
  cputs(" KB   Reserved: ");
  cputs(citoaul(ppa_get_mem_rsvd() / 1024, buff, 10));
  cputs(" KB   Free: ");
  cputs(citoaul(ppa_get_mem_free() / 1024, buff, 10));
  cputs(" KB   Total: ");
  cputs(citoaul(ppa_get_mem_total() / 1024, buff, 10));
  cputln(" KB.");

  cputln("requesting 20 pages...");
  for (int i = 0; i < 20; i++) {
    void *addr = ppa_request();
    cputln(citoaul((uint64_t)addr, buff, 16));
  }

  cputs("Used: ");
  cputs(citoaul(ppa_get_mem_used() / 1024, buff, 10));
  cputs(" KB   Reserved: ");
  cputs(citoaul(ppa_get_mem_rsvd() / 1024, buff, 10));
  cputs(" KB   Free: ");
  cputs(citoaul(ppa_get_mem_free() / 1024, buff, 10));
  cputs(" KB   Total: ");
  cputs(citoaul(ppa_get_mem_total() / 1024, buff, 10));
  cputln(" KB.");

  cputln("Done");
  for (;;)
    ;
}