OSNAME:=slowos
OSIMG:=$(OSNAME).img

ifeq ($(shell uname),Darwin)
	ARCH=x86_64
	MAKE=gmake
	CC=x86_64-unknown-linux-gnu-gcc
	AR=x86_64-unknown-linux-gnu-ar
	LD=x86_64-unknown-linux-gnu-ld
	OBJCOPY=x86_64-unknown-linux-gnu-objcopy
else
	OBJCOPY=objcopy
endif

CFLAGS=-Ignu-efi/inc \
	-fpic -ffreestanding \
	-fno-stack-protector \
	-fno-stack-check \
	-mno-red-zone \
	-fshort-wchar \
	-maccumulate-outgoing-args

LDFLAGS=-shared -Bsymbolic \
	-Lgnu-efi/x86_64/lib \
	-Lgnu-efi/x86_64/gnuefi \
	-Tgnu-efi/gnuefi/elf_x86_64_efi.lds \
	gnu-efi/x86_64/gnuefi/crt0-efi-x86_64.o

LOADLIBES=-lgnuefi -lefi
LDLIBS=$(LOADLIBES)

.PHONY: all
all: $(OSIMG)

gnu-efi/%: libgnuefi

.PHONY: libgnuefi
libgnuefi:
	$(MAKE) -C gnu-efi ARCH=$(ARCH) MAKE=$(MAKE) CC=$(CC) AR=$(AR) LD=$(LD)

$(OSIMG): bootloader.efi kernel.elf
	dd if=/dev/zero of=$(OSIMG) bs=512 count=93750
	mformat -i $(OSIMG) ::
	mmd -i $(OSIMG) ::/EFI
	mmd -i $(OSIMG) ::/EFI/BOOT
	mcopy -i $(OSIMG) kernel.elf ::
	mcopy -i $(OSIMG) bootloader.efi ::/EFI/BOOT/BOOTX64.EFI

kernel.elf: kernel.o kernel.ld font.o conlib.o efimem.o
	$(LD) -T $(word 2,$^) -static -Bsymbolic -nostdlib -o kernel.elf kernel.o font.o conlib.o efimem.o

font.o: u_vga16.sfn
	$(LD) -r -b binary -o $@ $<

%.elf: %.o %.ld
	$(LD) -T $(word 2,$^) -static -Bsymbolic -nostdlib -o $@ $<

%.efi: %.so
	$(OBJCOPY) -j .text \
		-j .sdata -j .data \
		-j .dynamic -j .dynsym  \
		-j .rel -j .rela \
		-j .rel.* -j .rela.* \
		-j .reloc \
		--target efi-app-x86_64 \
		--subsystem=10 \
		$< $@

%.so: %.o libgnuefi
	$(LD) $(LDFLAGS) $< -o $@ $(LDLIBS) 

qemu: $(OSIMG)
	qemu-system-x86_64 \
		-m 256M -cpu qemu64 -net none \
		-drive "file=$(OSIMG),format=raw" \
		-drive if=pflash,format=raw,unit=0,file=OVMF_CODE.fd,readonly=on\
		-drive if=pflash,format=raw,unit=1,file=OVMF_VARS.fd

.PHONY: clean
clean:
	$(MAKE) -C gnu-efi clean
	rm -f $(OSIMG) *.so bootloader.efi kernel.elf *.o

# .SECONDARY:
