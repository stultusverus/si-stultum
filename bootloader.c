#ifndef __x86_64__
#define __x86_64__ 1
#endif
#undef __ILP32__ // for vscode... supposed to be doing nothing.

#include <efi.h>
#include <efilib.h>
#include <elf.h>
#include <libsmbios.h>

#include "kernel.h"

typedef void (*KERNEL_ENTRY)(BootInfo);

const CHAR16 *KERNEL_FILENAME = (CHAR16 *)L"kernel.elf";

static EFI_STATUS PrintSystemInfo() {
  // Function from pbatard/uefi-simple
  EFI_STATUS Status;
  SMBIOS_STRUCTURE_POINTER Smbios;
  SMBIOS_STRUCTURE_TABLE *SmbiosTable;
  SMBIOS3_STRUCTURE_TABLE *Smbios3Table;
  UINT8 Found = 0, *Raw, *SecureBoot, *SetupMode;
  UINTN MaximumSize, ProcessedSize = 0;

  Print(L"UEFI v%d.%d (%s, 0x%08X)\n", gST->Hdr.Revision >> 16,
        gST->Hdr.Revision & 0xFFFF, gST->FirmwareVendor, gST->FirmwareRevision);

  Status =
      LibGetSystemConfigurationTable(&SMBIOS3TableGuid, (VOID **)&Smbios3Table);
  if (Status == EFI_SUCCESS) {
    Smbios.Hdr = (SMBIOS_HEADER *)Smbios3Table->TableAddress;
    MaximumSize = (UINTN)Smbios3Table->TableMaximumSize;
  } else {
    Status =
        LibGetSystemConfigurationTable(&SMBIOSTableGuid, (VOID **)&SmbiosTable);
    if (EFI_ERROR(Status))
      return EFI_NOT_FOUND;
    Smbios.Hdr = (SMBIOS_HEADER *)(UINTN)SmbiosTable->TableAddress;
    MaximumSize = (UINTN)SmbiosTable->TableLength;
  }

  while ((Smbios.Hdr->Type != 0x7F) && (Found < 2)) {
    Raw = Smbios.Raw;
    if (Smbios.Hdr->Type == 0) {
      Print(L"%a %a\n", LibGetSmbiosString(&Smbios, Smbios.Type0->Vendor),
            LibGetSmbiosString(&Smbios, Smbios.Type0->BiosVersion));
      Found++;
    }
    if (Smbios.Hdr->Type == 1) {
      Print(L"%a %a\n", LibGetSmbiosString(&Smbios, Smbios.Type1->Manufacturer),
            LibGetSmbiosString(&Smbios, Smbios.Type1->ProductName));
      Found++;
    }
    LibGetSmbiosString(&Smbios, -1);
    ProcessedSize += (UINTN)Smbios.Raw - (UINTN)Raw;
    if (ProcessedSize > MaximumSize) {
      Print(L"%EAborting system report due to noncompliant SMBIOS%N\n");
      return EFI_ABORTED;
    }
  }

  SecureBoot = LibGetVariable(L"SecureBoot", &EfiGlobalVariable);
  SetupMode = LibGetVariable(L"SetupMode", &EfiGlobalVariable);
  UINT8 SecureBootStatus = ((SecureBoot != NULL) && (*SecureBoot != 0)) ? 1 : 0;
  // You'd expect UEFI platforms to properly clear SetupMode after they
  // installed all the certs... but most of them don't. Hence Secure Boot
  // disabled having precedence over SetupMode. Looking at you OVMF!
  if ((SetupMode != NULL) && (*SetupMode != 0))
    SecureBootStatus *= -1;
  // Wasteful, but we can't highlight "Enabled"/"Setup" from a %s argument...
  if (SecureBootStatus > 0)
    Print(L"Secure Boot status: %HEnabled%N\n");
  else if (SecureBootStatus < 0)
    Print(L"Secure Boot status: %ESetup%N\n");
  else
    Print(L"Secure Boot status: Disabled\n");

  return EFI_SUCCESS;
}

EFI_FILE_HANDLE GetVolume(EFI_HANDLE Image) {
  EFI_FILE_HANDLE Volume;
  EFI_LOADED_IMAGE *LoadedImage;
  EFI_GUID LipGuid = EFI_LOADED_IMAGE_PROTOCOL_GUID;
  EFI_FILE_IO_INTERFACE *IOVolume;
  EFI_GUID FileSystemGuid = EFI_SIMPLE_FILE_SYSTEM_PROTOCOL_GUID;
  uefi_call_wrapper(BS->HandleProtocol, 3, Image, &LipGuid,
                    (void **)&LoadedImage);
  Volume = LibOpenRoot(LoadedImage->DeviceHandle);
  return Volume;
}

UINT64 GetFileSize(EFI_FILE_HANDLE FileHandle) {
  UINT64 FileSize;
  EFI_FILE_INFO *FileInfo = LibFileInfo(FileHandle);
  FileSize = FileInfo->FileSize;
  FreePool(FileInfo);
  return FileSize;
}

EFI_GRAPHICS_OUTPUT_PROTOCOL *GetGop() {
  EFI_GRAPHICS_OUTPUT_PROTOCOL *Gop;
  EFI_STATUS Status;
  EFI_GUID GopGuid = EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID;
  Status =
      uefi_call_wrapper(BS->LocateProtocol, 3, &GopGuid, NULL, (void **)&Gop);
  if (EFI_ERROR(Status))
    return NULL;
  return Gop;
}

EFI_STATUS InitializeGop(FrameBuffer *frameBuffer) {
  EFI_GRAPHICS_OUTPUT_PROTOCOL *Gop;
  EFI_STATUS Status;
  EFI_GRAPHICS_OUTPUT_MODE_INFORMATION *Info;
  UINTN SizeOfInfo, NumModes, NativeMode;

  Gop = GetGop();
  if (Gop == NULL)
    return EFI_NOT_FOUND;

  // activate native mode.
  Status = uefi_call_wrapper(Gop->QueryMode, 4, Gop,
                             Gop->Mode == NULL ? 0 : Gop->Mode->Mode,
                             &SizeOfInfo, &Info);
  if (Status == EFI_NOT_STARTED)
    Status = uefi_call_wrapper(Gop->SetMode, 2, Gop, 0);
  if (EFI_ERROR(Status))
    return Status;

  frameBuffer->fbbase = Gop->Mode->FrameBufferBase;
  frameBuffer->fbsize = Gop->Mode->FrameBufferSize;
  frameBuffer->height = Gop->Mode->Info->VerticalResolution;
  frameBuffer->width = Gop->Mode->Info->HorizontalResolution;
  frameBuffer->ppsl = Gop->Mode->Info->PixelsPerScanLine;

  return EFI_SUCCESS;
}

EFI_MEMORY_DESCRIPTOR *GetMemoryMap(UINTN *MapSize, UINTN *DescriptorSize) {
  EFI_STATUS Status;
  EFI_MEMORY_DESCRIPTOR *MemoryMap = NULL;
  UINT32 DescriptorVersion;
  UINTN MapKey;
  *MapSize = sizeof(EFI_MEMORY_DESCRIPTOR);
  do {
    MemoryMap = AllocatePool(*MapSize);
    Status = uefi_call_wrapper(BS->GetMemoryMap, 5, MapSize, MemoryMap, &MapKey,
                               DescriptorSize, &DescriptorVersion);
    if (EFI_ERROR(Status) && Status != EFI_BUFFER_TOO_SMALL)
      return NULL;
  } while (Status == EFI_BUFFER_TOO_SMALL);
  return MemoryMap;
}

KERNEL_ENTRY GetKernelEntry(EFI_HANDLE ImageHandle, EFI_STATUS *Status) {
  EFI_FILE_HANDLE Volume = GetVolume(ImageHandle);
  EFI_FILE_HANDLE KernelFile;
  uefi_call_wrapper(Volume->Open, 5, Volume, &KernelFile, KERNEL_FILENAME,
                    EFI_FILE_MODE_READ,
                    EFI_FILE_READ_ONLY | EFI_FILE_HIDDEN | EFI_FILE_SYSTEM);

  Elf64_Ehdr Headers;
  UINT64 HeaderSize = sizeof(Headers);
  uefi_call_wrapper(KernelFile->Read, 3, KernelFile, &HeaderSize, &Headers);
  if (strncmpa(&Headers.e_ident[EI_MAG0], ELFMAG, SELFMAG) ||
      Headers.e_ident[EI_CLASS] != ELFCLASS64 ||
      Headers.e_ident[EI_DATA] != ELFDATA2LSB || Headers.e_type != ET_EXEC ||
      Headers.e_machine != EM_X86_64 || Headers.e_version != EV_CURRENT) {
    *Status = EFI_LOAD_ERROR;
    return NULL;
  }

  Elf64_Phdr *ProgramHeaders; // pointer to the 1st element

  UINT64 LoadSize = Headers.e_phnum * Headers.e_phentsize;
  uefi_call_wrapper(ST->BootServices->AllocatePool, 3, EfiLoaderData, LoadSize,
                    &ProgramHeaders);
  uefi_call_wrapper(KernelFile->SetPosition, 2, KernelFile, Headers.e_phoff);
  uefi_call_wrapper(KernelFile->Read, 3, KernelFile, &LoadSize, ProgramHeaders);

  for (int i = 0; i < Headers.e_phnum; i++) {
    if (ProgramHeaders[i].p_type == PT_LOAD) {
      UINTN Pages = (ProgramHeaders[i].p_memsz + 0x1000 - 1) / 0x1000;
      Elf64_Addr Segment = ProgramHeaders[i].p_paddr;
      UINT64 SegmentSize = ProgramHeaders[i].p_filesz;
      uefi_call_wrapper(ST->BootServices->AllocatePages, 3, AllocateAddress,
                        EfiLoaderData, Pages, &Segment);
      uefi_call_wrapper(KernelFile->SetPosition, 2, KernelFile,
                        ProgramHeaders[i].p_offset);
      uefi_call_wrapper(KernelFile->Read, 3, KernelFile, &SegmentSize, Segment);
    }
  }
  return ((__attribute__((sysv_abi)) void (*)(BootInfo))Headers.e_entry);
}

EFI_STATUS
EFIAPI
efi_main(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable) {
  EFI_STATUS Status;
  FrameBuffer frameBuffer;

  InitializeLib(ImageHandle, SystemTable);
  uefi_call_wrapper(ST->ConOut->ClearScreen, 1, ST->ConOut);
  PrintSystemInfo();

  Status = InitializeGop(&frameBuffer);
  if (EFI_ERROR(Status)) {
    Print(L"Unable to initialize GOP.\n\r");
    goto END;
  }
  Print(L"Framebuffer address 0x%08x, SIZE=%d, %dx%d PPSL=%d\n\r",
        frameBuffer.fbbase, frameBuffer.fbsize, frameBuffer.width,
        frameBuffer.height, frameBuffer.ppsl);

  EFI_MEMORY_DESCRIPTOR *EfiMemoryMap = NULL;
  UINTN EfiMemoryMapSize = 0;
  UINTN EfiMemoryMapDescriptorSize = sizeof(EFI_MEMORY_DESCRIPTOR);
  UINT32 EfiMemoriMapDescriptorVersion;

  EfiMemoryMap = GetMemoryMap(&EfiMemoryMapSize, &EfiMemoryMapDescriptorSize);
  if (EfiMemoryMap == NULL) {
    Print(L"Unable to load memory map.\n\r");
    goto END;
  }

  Print(L"EFI MEMORY MAP LOADED, GOING TO KERNEL...\n\r");
  // for (int i = 3; i > 0; i--) {
  //   Print(L"%02d\r", i);
  //   uefi_call_wrapper(BS->Stall, 1, 1000000);
  // }
  uefi_call_wrapper(ST->ConOut->ClearScreen, 1, ST->ConOut);

  KERNEL_ENTRY KernelEntry = GetKernelEntry(ImageHandle, &Status);
  if (EFI_ERROR(Status)) {
    Print(L"Unable to find kernel entry.\n\r");
    goto END;
  }

  KernelEntry((BootInfo){.fb = &frameBuffer,
                         .mmap = (EfiMemoryDescriptor *)EfiMemoryMap,
                         .mmap_size = EfiMemoryMapSize,
                         .mmap_desc_size = EfiMemoryMapDescriptorSize});

END:
  for (;;)
    ;
  return EFI_SUCCESS;
}