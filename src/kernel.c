#include "kernel.h"
#include "chrono.h"
#include "conlib.h"
#include "gdt.h"
#include "idt.h"
#include "interrupts.h"
#include "page_frames.h"
#include "page_map.h"
#include "port_io.h"
#include "serial.h"

extern uint8_t _binary_assets_u_vga16_sfn_start;
extern uint8_t _binary_assets_u_vga16_sfn_end;
extern uint8_t _kernel_start;
extern uint8_t _kernel_end;

char buff[100];
const char print_str[] = "DIXITQVE DEVS FIAT LVX ET FACTA EST LVX";

GDTDescriptor GDTR;
IDTDescriptor IDTR;

void init_gdt() {
  GDTR = (GDTDescriptor){
      .size   = sizeof(GDT) - 1,
      .offset = (uint64_t)&DEFAULT_GDT,
  };
  ppa_memset((uint8_t *)&DEFAULT_GDT + 64 * 6, 0, 4096 - 64 * 6);
  set_gdt(&GDTR);
}

void init_pmm(uint64_t fbbase, uint64_t fbsize) {
  PageTable *PML4 = (PageTable *)ppa_request();
  ppa_memset(PML4, 0, 4096);
  pmm_init_pml4(PML4);
  for (uint64_t addr = 0; addr < ppa_get_mem_total(); addr += 4096)
    pmm_map_memory((void *)addr, (void *)addr);
  for (uint64_t addr = fbbase; addr < fbbase + fbsize; addr += 4096)
    pmm_map_memory((void *)addr, (void *)addr);
  set_pml4(PML4);
}

void init_idt() {
  IDTR = (IDTDescriptor){
      .offset = (uint64_t)ppa_request(),
      .size   = 0x0fff,
  };

  IDTEntry *int_page_fault = (IDTEntry *)(IDTR.offset + 0xE * sizeof(IDTEntry));
  *int_page_fault = IDT_CREATE_ENTRY((uint64_t)PageFault_Handler, 0x08, 0,
                                     IDT_INTERRUPT_GATE);

  IDTEntry *int_double_fault =
      (IDTEntry *)(IDTR.offset + 0x8 * sizeof(IDTEntry));
  *int_double_fault = IDT_CREATE_ENTRY((uint64_t)DoubleFault_Handler, 0x08, 0,
                                       IDT_INTERRUPT_GATE);

  IDTEntry *int_general_protection_fault =
      (IDTEntry *)(IDTR.offset + 0xD * sizeof(IDTEntry));
  *int_general_protection_fault = IDT_CREATE_ENTRY(
      (uint64_t)GeneralProtectionFault_Handler, 0x08, 0, IDT_INTERRUPT_GATE);

  IDTEntry *int_timer = (IDTEntry *)(IDTR.offset + 0x20 * sizeof(IDTEntry));
  *int_timer =
      IDT_CREATE_ENTRY((uint64_t)Timer_Handler, 0x08, 0, IDT_INTERRUPT_GATE);

  IDTEntry *int_keyboard = (IDTEntry *)(IDTR.offset + 0x21 * sizeof(IDTEntry));
  *int_keyboard =
      IDT_CREATE_ENTRY((uint64_t)Keyboard_Handler, 0x08, 0, IDT_INTERRUPT_GATE);

  set_idt(&IDTR);
  PIC_remap(0x20, 0x28);
  init_PIT(1000);
  outb(PIC1_DATA, 0xfc); // Master 8259 masks
  outb(PIC2_DATA, 0xff); // Slave 8259 masks
  enable_interrupts();
}

void init_kernel(BootInfo boot_info) {
  init_gdt();
  serial_printf("GDT loaded.\n");

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

  init_pmm(fbbase, fbsize);

  init_idt();
  serial_printf("IDT loaded.\n");
}

void _start(BootInfo boot_info) {

  init_serial();
  serial_printf("Serial logging enabled.\n");

  init_kernel(boot_info);

  serial_printf(
      "[MEM] %d KB USED   %d KB RESERVED   %d KB FREE   %d KB TOTAL\n",
      ppa_get_mem_used() / 1024, ppa_get_mem_rsvd() / 1024,
      ppa_get_mem_free() / 1024, ppa_get_mem_total() / 1024);

  serial_printf("Testing memory mapping...\n");
  char *vstr = (char *)0x600000000;
  pmm_map_memory(vstr, (void *)0x80000);
  ppa_memcpy(vstr, print_str, 40);
  serial_printf("%s\n", vstr);

  serial_printf("Memory mapping OK.\n");

  serial_printf("[MEM] %d KB USED   %d KB RESERVED   %d KB "
                "FREE   %d KB TOTAL\n",
                ppa_get_mem_used() / 1024, ppa_get_mem_rsvd() / 1024,
                ppa_get_mem_free() / 1024, ppa_get_mem_total() / 1024);

  set_color(BLACK, WHITE);
  puts("  WHITE   ");
  set_color(BLACK, YELLOW);
  puts("  YELLOW  ");
  set_color(BLACK, ORANGE);
  puts("  ORANGE  ");
  set_color(WHITE, RED);
  puts("   RED    ");
  set_color(WHITE, MAGENTA);
  puts(" MAGENTA  ");
  set_color(WHITE, PURPLE);
  puts("  PURPLE  ");
  set_color(WHITE, BLUE);
  puts("   BLUE   ");
  set_color(BLACK, CYAN);
  putln("   CYAN   ");
  set_color(WHITE, GREEN);
  puts("  GREEN   ");
  set_color(WHITE, DARKGREEN);
  puts("DARKGREEN ");
  set_color(WHITE, BROWN);
  puts("   BROWN  ");
  set_color(WHITE, TAN);
  puts("   TAN    ");
  set_color(BLACK, LIGHTGREY);
  puts("LIGHTGREY ");
  set_color(WHITE, MEDIUMGREY);
  puts("MEDIUMGREY");
  set_color(WHITE, DARKGREY);
  puts(" DARKGREY ");
  set_color(WHITE, BLACK);
  putln("  BLACK  \n");
  puts_at(-4, 0, "DONE");

  // serial_printf("Done. Shuting down...\n");
  // outw(0x604, 0x2000);  // newer version
  // outw(0xB004, 0x2000); // older version

  while (ms_count < 3000)
    asm("hlt");
  serial_printf("Now try typing something.\n");
  for (;;) {
    asm("hlt");
    if (serial_received()) {
      int ch = kgetc();
      kputc(ch);
    }
  }
  __builtin_unreachable();
}
