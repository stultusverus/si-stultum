OSNAME:=slowos
OSIMG:=$(OSNAME).img

SUBDIRS:=$(wildcard */.)

.PHONY: all qemu clean $(SUBDIRS)

all: $(SUBDIRS) $(OSIMG)

$(SUBDIRS):
	$(MAKE) -C $@

$(OSIMG): main.efi
	dd if=/dev/zero of=$(OSIMG) bs=512 count=93750
	mformat -i $(OSIMG) ::
	mmd -i $(OSIMG) ::/EFI
	mmd -i $(OSIMG) ::/EFI/BOOT
	mcopy -i $(OSIMG) main.efi ::/EFI/BOOT

main.efi: main.so
	objcopy -j .text \
		-j .sdata -j .data \
		-j .dynamic -j .dynsym  \
		-j .rel -j .rela \
		-j .rel.* -j .rela.* \
		-j .reloc \
		--target efi-app-x86_64 \
		--subsystem=10 \
		main.so main.efi

main.so: $(SUBDIRS) main.o
	ld -shared -Bsymbolic \
	-Lgnu-efi/x86_64/lib \
	-Lgnu-efi/x86_64/gnuefi \
	-Tgnu-efi/gnuefi/elf_x86_64_efi.lds \
	gnu-efi/x86_64/gnuefi/crt0-efi-x86_64.o \
	main.o -o main.so -lgnuefi -lefi

main.o: main.c
	gcc -Ignu-efi/inc \
	-fpic -ffreestanding \
	-fno-stack-protector \
	-fno-stack-check \
	-mno-red-zone \
	-fshort-wchar \
	-maccumulate-outgoing-args \
	-c main.c -o main.o

qemu: $(OSIMG) OVMF.fd
	qemu-system-x86_64 \
		-m 256M -cpu qemu64 -net none \
		-drive "file=$(OSIMG)" \
		-pflash ./OVMF.fd

clean:
	$(MAKE) -C $(SUBDIRS) clean
	rm -f $(OSIMG) *.so main.efi *.o