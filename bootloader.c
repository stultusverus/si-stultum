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
  Print(L"size is %d.\n\r", (INT32)get_file_size(kernel_file));
  Elf64_Ehdr header;
  {}
  for (;;)
    ;
  return EFI_SUCCESS;
}
