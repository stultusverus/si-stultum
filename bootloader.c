#include <efi.h>
#include <efilib.h>

EFI_STATUS
EFIAPI
efi_main(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable) {
  InitializeLib(ImageHandle, SystemTable);
  uefi_call_wrapper(ST->ConOut->OutputString, 2, ST->ConOut,
                    L"Hello, world!\n\r");
  Print(L"%s : %d\n", SystemTable->FirmwareVendor,
        SystemTable->FirmwareRevision);
  for (;;)
    ;
  return EFI_SUCCESS;
}
