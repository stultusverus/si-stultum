#include "kernel.h"
#include "conlib.h"
#include "gdt.h"
#include "idt.h"
#include "interrupts.h"
#include "page_frames.h"
#include "page_map.h"

extern uint8_t _binary_assets_u_vga16_sfn_start;
extern uint8_t _binary_assets_u_vga16_sfn_end;
extern uint8_t _kernel_start;
extern uint8_t _kernel_end;

void clear_interrupts();
void set_pml4(PageTable *);

char buff[100];
const char print_str[] = "DIXITQVE DEVS FIAT LVX ET FACTA EST LVX";

GDTDescriptor GDTR;
IDTDescriptor IDTR;

void init_kernel(BootInfo boot_info) {
  /* initialise GDT */
  GDTR = (GDTDescriptor){
      .size = sizeof(GDT) - 1,
      .offset = (uint64_t)&DEFAULT_GDT,
  };
  ppa_memset((uint8_t *)&DEFAULT_GDT + 64 * 6, 0, 4096 - 64 * 6);
  set_gdt(&GDTR);
  /* initialise console output */
  conlib_init((ssfn_font_t *)&_binary_assets_u_vga16_sfn_start,
              (void *)boot_info.fb->fbbase, boot_info.fb->width,
              boot_info.fb->height, boot_info.fb->ppsl * 4);
  /* initialise page allocator */
  uint64_t kernel_size = (uint64_t)&_kernel_end - (uint64_t)&_kernel_start;
  uint64_t kernel_pages =
      kernel_size / 4096 + (kernel_size % 4096 != 0 ? 1 : 0);
  uint64_t fbbase = (uint64_t)boot_info.fb->fbbase;
  uint64_t fbsize = (uint64_t)boot_info.fb->fbsize + 4096;
  ppa_init(boot_info.mmap, boot_info.mmap_size, boot_info.mmap_desc_size);
  ppa_lckn(&_kernel_start, kernel_pages);
  ppa_lckn((void *)fbbase, fbsize / 4096);
  /* initialise page table */
  PageTable *PML4 = (PageTable *)ppa_request();
  ppa_memset(PML4, 0, 4096);
  pmm_init_pml4(PML4);
  for (uint64_t addr = 0; addr < ppa_get_mem_total(); addr += 4096)
    pmm_map_memory((void *)addr, (void *)addr);
  for (uint64_t addr = fbbase; addr < fbbase + fbsize; addr += 4096)
    pmm_map_memory((void *)addr, (void *)addr);
  set_pml4(PML4);
  /* initialise IDT */
  IDTR = (IDTDescriptor){
      .offset = (uint64_t)ppa_request(),
      .size = 0x0fff,
  };
  IDTEntry *int_page_fault = (IDTEntry *)(IDTR.offset + 0xE * sizeof(IDTEntry));
  *int_page_fault = IDT_CREATE_ENTRY((uint64_t)PageFault_Handler, 0x08, 0,
                                     IDT_INTERRUPT_GATE);

  asm("lidt %0" : : "m"(IDTR));

  // clear_interrupts();
}

void _start(BootInfo boot_info) {

  init_kernel(boot_info);

  puts("[MEM] Used: ");
  puts(itoa(ppa_get_mem_used() / 1024, buff, 10));
  puts(" KB   Reserved: ");
  puts(itoa(ppa_get_mem_rsvd() / 1024, buff, 10));
  puts(" KB   Free: ");
  puts(itoa(ppa_get_mem_free() / 1024, buff, 10));
  puts(" KB   Total: ");
  puts(itoa(ppa_get_mem_total() / 1024, buff, 10));
  putln(" KB.");

  char *vstr = (char *)0x600000000;
  pmm_map_memory(vstr, (void *)0x80000);
  ppa_memcpy(vstr, print_str, 40);
  putln(vstr);

  puts_n(9, "[MEM] Used: ", itoa(ppa_get_mem_used() / 1024, buff, 10),
         " KB   Reserved: ", itoa(ppa_get_mem_rsvd() / 1024, buff, 10),
         " KB   Free: ", itoa(ppa_get_mem_free() / 1024, buff, 10),
         " KB   Total: ", itoa(ppa_get_mem_total() / 1024, buff, 10), " KB.\n");

  set_color(BLACK, WHITE);
  putln("  WHITE   ");
  set_color(BLACK, YELLOW);
  putln("  YELLOW  ");
  set_color(BLACK, ORANGE);
  putln("  ORANGE  ");
  set_color(WHITE, RED);
  putln("   RED    ");
  set_color(WHITE, MAGENTA);
  putln(" MAGENTA  ");
  set_color(WHITE, PURPLE);
  putln("  PURPLE  ");
  set_color(WHITE, BLUE);
  putln("   BLUE   ");
  set_color(BLACK, CYAN);
  putln("   CYAN   ");
  set_color(WHITE, GREEN);
  putln("  GREEN   ");
  set_color(WHITE, DARKGREEN);
  putln("DARKGREEN ");
  set_color(WHITE, BROWN);
  putln("   BROWN  ");
  set_color(WHITE, TAN);
  putln("   TAN    ");
  set_color(BLACK, LIGHTGREY);
  putln("LIGHTGREY ");
  set_color(WHITE, MEDIUMGREY);
  putln("MEDIUMGREY");
  set_color(WHITE, DARKGREY);
  putln(" DARKGREY ");
  set_color(WHITE, BLACK);
  putln("  BLACK   ");

  puts_at(-4, 0, "DONE");
  putln("\n\nDONE.");
  for (;;)
    ;
}
