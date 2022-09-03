SRCDIR=src
OBJDIR=obj
IMGDIR=img
OSNAME=si-stultum
OSIMG=$(IMGDIR)/$(OSNAME).img

ARCH=x86_64

ifeq ($(shell uname),Darwin)
	MAKE=gmake
	CC=x86_64-unknown-linux-gnu-gcc
	AR=x86_64-unknown-linux-gnu-ar
	LD=x86_64-unknown-linux-gnu-ld
	OBJCOPY=x86_64-unknown-linux-gnu-objcopy
else
	OBJCOPY=objcopy
	MAKE=make
endif

ASM=nasm

CFLAGS=-Ignu-efi/inc \
	-fpic -ffreestanding \
	-fno-stack-protector \
	-fno-stack-check \
	-mno-red-zone \
	-fshort-wchar \
	-maccumulate-outgoing-args \
	-Werror -Wall -O3

LDFLAGS=-shared -Bsymbolic \
	-Lgnu-efi/x86_64/lib \
	-Lgnu-efi/x86_64/gnuefi \
	-Tgnu-efi/gnuefi/elf_x86_64_efi.lds \
	gnu-efi/x86_64/gnuefi/crt0-efi-x86_64.o

LOADLIBES=-lgnuefi -lefi
LDLIBS=$(LOADLIBES)

KERNEL_TARGETS=kernel font conlib efimem page_frames page_map gdt
KERNEL_OBJS=$(patsubst %,obj/%.o,$(KERNEL_TARGETS))

.PHONY: all
all: $(OSIMG)

$(OSIMG): $(OBJDIR)/bootloader.efi $(OBJDIR)/kernel.elf
	if [ ! -f $(OSIMG) ];then \
		mkdir -p $(IMGDIR); \
		dd if=/dev/zero of=$(OSIMG) bs=512 count=93750; \
	fi
	mformat -i $(OSIMG) ::
	mmd -i $(OSIMG) ::/EFI
	mmd -i $(OSIMG) ::/EFI/BOOT
	mcopy -i $(OSIMG) $(OBJDIR)/kernel.elf ::
	mcopy -i $(OSIMG) $(OBJDIR)/bootloader.efi ::/EFI/BOOT/BOOTX64.EFI

$(OBJDIR):
	mkdir -p $@

$(OBJDIR)/kernel.elf: $(SRCDIR)/kernel.ld $(KERNEL_OBJS) $(OBJDIR)/asm_utils.o $(OBJDIR)
	$(LD) -T $< -static -Bsymbolic -nostdlib -o $@ $(OBJDIR)/asm_utils.o $(KERNEL_OBJS)

$(OBJDIR)/asm_utils.o: $(SRCDIR)/asm_utils.asm
	$(ASM) $(ASFLAGS) $< -f elf64 -o $@

$(OBJDIR)/font.o: assets/u_vga16.sfn  $(OBJDIR)
	$(LD) -r -b binary -o $@ $<

$(OBJDIR)/%.efi: $(OBJDIR)/%.so  $(OBJDIR)
	$(OBJCOPY) -j .text \
		-j .sdata -j .data \
		-j .dynamic -j .dynsym  \
		-j .rel -j .rela \
		-j .rel.* -j .rela.* \
		-j .reloc \
		--target efi-app-x86_64 \
		--subsystem=10 \
		$< $@

$(OBJDIR)/%.so: $(OBJDIR)/%.o  $(OBJDIR)
	if [ ! -f gnu-efi/x86_64/gnuefi/crt0-efi-x86_64.o ];then \
		$(MAKE) -C gnu-efi ARCH=$(ARCH) MAKE=$(MAKE) CC=$(CC) AR=$(AR) LD=$(LD) ;\
	fi
	$(LD) $(LDFLAGS) $< -o $@ $(LDLIBS) 

$(OBJDIR)/%.o: $(SRCDIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

qemu: $(OSIMG)
	qemu-system-x86_64 \
		-m 256M -cpu qemu64 -net none \
		-drive "file=$(OSIMG),format=raw" \
		-drive if=pflash,format=raw,unit=0,file=OVMF/OVMF_CODE.fd,readonly=on\
		-drive if=pflash,format=raw,unit=1,file=OVMF/OVMF_VARS.fd

qemu-debug: $(OSIMG)
	qemu-system-x86_64 -S -s\
		-m 256M -cpu qemu64 -net none \
		-drive "file=$(OSIMG),format=raw" \
		-drive if=pflash,format=raw,unit=0,file=OVMF/OVMF_CODE.fd,readonly=on\
		-drive if=pflash,format=raw,unit=1,file=OVMF/OVMF_VARS.fd

.PHONY: clean
clean:
	$(MAKE) -C gnu-efi clean
	rm -rf $(OBJDIR)
	rm -rf $(IMGDIR)

