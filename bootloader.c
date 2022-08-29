#include <efi.h>
#include <efilib.h>
#include <elf.h>

EFI_FILE_HANDLE get_volume(EFI_HANDLE image) {
  EFI_FILE_HANDLE volume;
  EFI_LOADED_IMAGE *loaded_image;
  EFI_GUID lip_guid = EFI_LOADED_IMAGE_PROTOCOL_GUID;
  EFI_FILE_IO_INTERFACE *io_volume;
  EFI_GUID fs_guid = EFI_SIMPLE_FILE_SYSTEM_PROTOCOL_GUID;
  uefi_call_wrapper(BS->HandleProtocol, 3, image, &lip_guid,
                    (void **)&loaded_image);
#ifdef _GNU_EFI
  volume = LibOpenRoot(loaded_image->DeviceHandle);
#else
  uefi_call_wrapper(BS->HandleProtocol, 3, loaded_image->DeviceHandle, &fs_guid,
                    (VOID *)&io_volume);
  uefi_call_wrapper(io_volume->OpenVolume, 2, io_volume, &volume);
#endif
  return volume;
}

UINT64 get_file_size(EFI_FILE_HANDLE file_handle) {
  UINT64 ret;
  EFI_FILE_INFO *file_info = LibFileInfo(file_handle);
  ret = file_info->FileSize;
  FreePool(file_info);
  return ret;
}

EFI_STATUS
EFIAPI
efi_main(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable) {
  InitializeLib(ImageHandle, SystemTable);
  uefi_call_wrapper(ST->ConOut->OutputString, 2, ST->ConOut,
                    L"Hello, world!\n\r");
  Print(L"%s : %d\n", SystemTable->FirmwareVendor,
        SystemTable->FirmwareRevision);
  EFI_FILE_HANDLE volume = get_volume(ImageHandle);
  EFI_FILE_HANDLE kernel_file;
  uefi_call_wrapper(volume->Open, 5, volume, &kernel_file, L"kernel.elf",
                    EFI_FILE_MODE_READ,
                    EFI_FILE_READ_ONLY | EFI_FILE_HIDDEN | EFI_FILE_SYSTEM);
  Elf64_Ehdr header;
  {
    UINT64 header_size = sizeof(header);
    uefi_call_wrapper(kernel_file->Read, 3, kernel_file, &header_size, &header);
  }
  int ok = 1;
  if (strncmpa(&header.e_ident[EI_MAG0], ELFMAG, SELFMAG)) {
    Print(L"EI_MAG0 ERROR\n\r");
    ok &= 0;
  }
  if (header.e_ident[EI_CLASS] != ELFCLASS64) {
    Print(L"EI_CLASS ERROR\n\r");
    ok &= 0;
  }
  if (header.e_ident[EI_DATA] != ELFDATA2LSB) {
    Print(L"EI_DATA ERROR\n\r");
    ok &= 0;
  }
  if (header.e_type != ET_EXEC) {
    Print(L"Non-Exec\n\r");
    ok &= 0;
  }
  if (header.e_machine != EM_X86_64) {
    Print(L"Wrong Machine\n\r");
    ok &= 0;
  }
  if (header.e_version != EV_CURRENT) {
    Print(L"Wrong version\n\r");
    ok &= 0;
  }
  if (ok)
    Print(L"Kernel header verified!\n\r");
  else
    goto END;
  Elf64_Phdr *program_headers;
  {
    UINT64 load_size = header.e_phnum * header.e_phentsize;
    uefi_call_wrapper(ST->BootServices->AllocatePool, 3, EfiLoaderData,
                      load_size, &program_headers);
    uefi_call_wrapper(kernel_file->SetPosition, 2, kernel_file, header.e_phoff);
    uefi_call_wrapper(kernel_file->Read, 3, kernel_file, &load_size,
                      program_headers);
  }
  for (int i = 0; i < header.e_phnum; i++) {
    Print(L"i=%d, e_phnum=%d ", i, header.e_phnum);
    switch (program_headers[i].p_type) {
    case PT_LOAD: {
      Print(L"[PT_LOAD]\n\r");
      int pages = (program_headers[i].p_memsz + 0x1000 - 1) / 0x1000;
      Elf64_Addr segment = program_headers[i].p_paddr;
      uefi_call_wrapper(ST->BootServices->AllocatePages, 3, AllocateAddress,
                        EfiLoaderData, pages, &segment);
      uefi_call_wrapper(kernel_file->SetPosition, 2, kernel_file,
                        program_headers[i].p_offset);
      UINT64 segment_size = program_headers[i].p_filesz;
      uefi_call_wrapper(kernel_file->Read, 3, kernel_file, &segment_size,
                        segment);
    } break;
    default:
      Print(L"[unknown]\n\r");
      break;
    }
  }
  Print(L"Done loading kernel.\n\r");

  int (*kernel_entry)() = ((__attribute__((sysv_abi)) int (*)())header.e_entry);
  Print(L"%d\n\r", kernel_entry());

END:
  for (;;)
    ;
  return EFI_SUCCESS;
}
