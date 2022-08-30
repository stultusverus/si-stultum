#include <efi.h>
#include <efilib.h>
#include <elf.h>

#include "kernel.h"

FrameBuffer fb;

EFI_FILE_HANDLE get_volume(EFI_HANDLE image) {
  EFI_FILE_HANDLE volume;
  EFI_LOADED_IMAGE *loaded_image;
  EFI_GUID lip_guid = EFI_LOADED_IMAGE_PROTOCOL_GUID;
  EFI_FILE_IO_INTERFACE *io_volume;
  EFI_GUID fs_guid = EFI_SIMPLE_FILE_SYSTEM_PROTOCOL_GUID;
  uefi_call_wrapper(BS->HandleProtocol, 3, image, &lip_guid,
                    (void **)&loaded_image);
  volume = LibOpenRoot(loaded_image->DeviceHandle);
  return volume;
}

UINT64 get_file_size(EFI_FILE_HANDLE file_handle) {
  UINT64 ret;
  EFI_FILE_INFO *file_info = LibFileInfo(file_handle);
  ret = file_info->FileSize;
  FreePool(file_info);
  return ret;
}

EFI_STATUS initialize_gop() {
  EFI_GRAPHICS_OUTPUT_PROTOCOL *gop;
  EFI_STATUS status;
  EFI_GUID gopGuid = EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID;
  EFI_GRAPHICS_OUTPUT_MODE_INFORMATION *info;
  UINTN SizeOfInfo, numModes, nativeMode;
  status =
      uefi_call_wrapper(BS->LocateProtocol, 3, &gopGuid, NULL, (void **)&gop);
  if (EFI_ERROR(status))
    return status;
  status = uefi_call_wrapper(gop->QueryMode, 4, gop,
                             gop->Mode == NULL ? 0 : gop->Mode->Mode,
                             &SizeOfInfo, &info);
  // this is needed to get the current video mode
  if (status == EFI_NOT_STARTED)
    status = uefi_call_wrapper(gop->SetMode, 2, gop, 0);
  if (EFI_ERROR(status))
    return status;
  nativeMode = gop->Mode->Mode;
  numModes = gop->Mode->MaxMode;
  for (int i = 0; i < numModes; i++) {
    uefi_call_wrapper(gop->QueryMode, 4, gop, i, &SizeOfInfo, &info);
    Print(L"mode %03d width %d height %d format %x%s\n\r", i,
          info->HorizontalResolution, info->VerticalResolution,
          info->PixelFormat, i == nativeMode ? "(current)" : "");
  }
  UINTN mode = 0;
  status = uefi_call_wrapper(gop->SetMode, 2, gop, mode);
  if (EFI_ERROR(status))
    return status;
  fb.fbbase = gop->Mode->FrameBufferBase;
  fb.fbsize = gop->Mode->FrameBufferSize;
  fb.height = gop->Mode->Info->VerticalResolution;
  fb.width = gop->Mode->Info->HorizontalResolution;
  fb.ppsl = gop->Mode->Info->PixelsPerScanLine;
  return EFI_SUCCESS;
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
  if (strncmpa(&header.e_ident[EI_MAG0], ELFMAG, SELFMAG) ||
      header.e_ident[EI_CLASS] != ELFCLASS64 ||
      header.e_ident[EI_DATA] != ELFDATA2LSB || header.e_type != ET_EXEC ||
      header.e_machine != EM_X86_64 || header.e_version != EV_CURRENT) {
    Print(L"Unable to verify kernel header.\n\r");
    goto END;
  } else
    Print(L"Kernel header verified!\n\r");
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
  Print(L"Initializing GOP...\n\r");

  EFI_STATUS status = initialize_gop();
  if (EFI_ERROR(status)) {
    Print(L"Unable to initialize GOP.\n\r");
    goto END;
  }
  Print(L"Initialized GOP.\n\rFramebuffer address 0x%x size %d, width %d "
        L"height %d pixelsperline %d\n\r",
        fb.fbbase, fb.fbsize, fb.width, fb.height, fb.ppsl);

  EFI_MEMORY_DESCRIPTOR *efi_memory_map = NULL;
  UINTN efi_memory_map_size = 0; // MUST!
  UINTN efi_map_key;
  UINTN efi_descriptor_size = sizeof(EFI_MEMORY_DESCRIPTOR);
  UINT32 efi_descriptor_version;

  uefi_call_wrapper(BS->GetMemoryMap, 5, &efi_memory_map_size, efi_memory_map,
                    &efi_map_key, &efi_descriptor_size,
                    &efi_descriptor_version);
  efi_memory_map = AllocatePool(efi_memory_map_size);
  status = uefi_call_wrapper(BS->GetMemoryMap, 5, &efi_memory_map_size,
                             efi_memory_map, &efi_map_key, &efi_descriptor_size,
                             &efi_descriptor_version);
  if (efi_memory_map == NULL ||
      EFI_ERROR(status) && status != EFI_BUFFER_TOO_SMALL) {
    Print(L"Unable to load EFI Memory Map.\n\r");
    goto END;
  }

  Print(L"EFI MEMORY MAP LOADED, GOING TO KERNEL\n\r");
  for (int i = 3; i > 0; i--) {
    Print(L"%d\r", i);
    uefi_call_wrapper(BS->Stall, 1, 1000000);
  }
  uefi_call_wrapper(ST->ConOut->ClearScreen, 1, ST->ConOut);

  void (*kernel_entry)(BootInfo) =
      ((__attribute__((sysv_abi)) void (*)(BootInfo))header.e_entry);

  kernel_entry((BootInfo){.fb = &fb,
                          .mmap = efi_memory_map,
                          .mmap_size = efi_memory_map_size,
                          .mmap_desc_size = efi_descriptor_size});

END:
  for (;;)
    ;
  return EFI_SUCCESS;
}