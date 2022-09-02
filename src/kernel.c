#include "kernel.h"
#include "conlib.h"
#include "page_frames.h"
#include "page_map.h"

extern uint8_t _binary_assets_u_vga16_sfn_start;
extern uint8_t _kernel_start;
extern uint8_t _kernel_end;

char buff[100];
const char print_str[] = "DIXITQVE DEVS FIAT LVX ET FACTA EST LVX";

void init_kernel(BootInfo boot_info) {
  uint64_t kernel_size = (uint64_t)&_kernel_end - (uint64_t)&_kernel_start;
  uint64_t kernel_pages =
      kernel_size / 4096 + (kernel_size % 4096 != 0 ? 1 : 0);
  uint64_t fbbase = (uint64_t)boot_info.fb->fbbase;
  uint64_t fbsize = (uint64_t)boot_info.fb->fbsize + 4096;

  cinit((ssfn_font_t *)&_binary_assets_u_vga16_sfn_start,
        (void *)boot_info.fb->fbbase, boot_info.fb->width, boot_info.fb->height,
        boot_info.fb->ppsl * 4);

  ppa_init(boot_info.mmap, boot_info.mmap_size, boot_info.mmap_desc_size);
  ppa_lckn(&_kernel_start, kernel_pages);
  ppa_lckn((void *)fbbase, fbsize / 4096);

  PageTable *PML4 = (PageTable *)ppa_request();
  ppa_memset(PML4, 0, 4096);
  pmm_init_pml4(PML4);

  for (uint64_t addr = 0; addr < ppa_get_mem_total(); addr += 4096) {
    pmm_map_memory((void *)addr, (void *)addr);
  }

  for (uint64_t addr = fbbase; addr < fbbase + fbsize; addr += 4096) {
    pmm_map_memory((void *)addr, (void *)addr);
  }
  asm("mov %0, %%cr3" : : "r"(PML4));
}

void debug_func(uint64_t vaddr) {
  uint64_t paddr = (uint64_t)pmm_find_paddr((void *)vaddr);
  cputs(citoaul(vaddr, buff, 16));
  cputs(" -> ");
  cputs(citoaul(paddr, buff, 16));
  cputs(" | ");
}

void _start(BootInfo boot_info) {

  init_kernel(boot_info);

  cputs("[MEM] Used: ");
  cputs(citoaul(ppa_get_mem_used() / 1024, buff, 10));
  cputs(" KB   Reserved: ");
  cputs(citoaul(ppa_get_mem_rsvd() / 1024, buff, 10));
  cputs(" KB   Free: ");
  cputs(citoaul(ppa_get_mem_free() / 1024, buff, 10));
  cputs(" KB   Total: ");
  cputs(citoaul(ppa_get_mem_total() / 1024, buff, 10));
  cputln(" KB.");

  char *vstr = (char *)0x600000000;
  pmm_map_memory(vstr, (void *)0x80000);
  ppa_memcpy(vstr, print_str, 40);
  cputln(vstr);

  cputs("[MEM] Used: ");
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