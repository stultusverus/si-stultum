[bits 64]

global set_pml4
set_pml4:
    mov rax, 0x000ffffffffff000
    and rdi, rax
    mov cr3, rdi
    ret

global set_idt
set_idt:
		lidt [rdi]
		ret

global set_gdt
set_gdt:
   lgdt  [rdi]
   mov   ax, 0x10
   mov   ds, ax
   mov   es, ax
   mov   fs, ax
   mov   gs, ax
   mov   ss, ax
	 pop	 rdi
	 mov rax, 0x08
	 push	 rax
	 push  rdi
   retfq
