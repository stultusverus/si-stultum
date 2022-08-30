#include "kernel.h"
#include "conlib.h"
#include "kermem.h"

extern unsigned char _binary_u_vga16_sfn_start;

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
  cputln(citoaul(boot_info.mmap_desc_size, buff, 16));
  cputs("Total memory size is ");
  cputs(citoaul(get_memory_size(boot_info.mmap, boot_info.mmap_size,
                                boot_info.mmap_desc_size),
                buff, 10));
  cputln(" Bytes.");

  uint64_t max_size = 0, min_size = 1024000;
  for (EfiMemoryDescriptor *desc = boot_info.mmap;
       (uint8_t *)desc < (uint8_t *)boot_info.mmap + boot_info.mmap_size;
       desc = ((EfiMemoryDescriptor *)(((uint8_t *)desc) +
                                       boot_info.mmap_desc_size))) {
    uint64_t crt_size = desc->numberofpages * 4;
    if (crt_size > max_size)
      max_size = crt_size;
    if (crt_size < min_size)
      min_size = crt_size;
    // cputs("Descriptor ");
    // cputs(" @0x");
    // cputs(citoaul((uint64_t)desc, buff, 16));
    // cputs(":[type=");
    // cputs(citoa(desc->type, buff, 10));
    // cputs("] ");
    // cputs(EFI_MEMORY_TYPE_STRINGS[desc->type]);
    // cputs(", size=");
    // cputs(citoaul(desc->numberofpages * 4096 / 1024, buff, 10));
    // cputs("KiB, P ");
    // cputs(citoaul((uint64_t)desc->physicalstart, buff, 16));
    // cputs(" V ");
    // cputln(citoaul((uint64_t)desc->virtualstart, buff, 16));
  }
  cputln("max & min page frames (KB):");
  cputln(citoaul(max_size, buff, 10));
  cputln(citoaul(min_size, buff, 10));

  cputln("Done");
}