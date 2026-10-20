#include <Uefi.h>

#include <Library/DebugLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/UefiBootServicesTableLib.h>

EFI_STATUS
EFIAPI
MunchMemoryMapDxeEntryPoint(
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  EFI_STATUS             Status;
  EFI_MEMORY_DESCRIPTOR  *MemoryMap;
  EFI_MEMORY_DESCRIPTOR  *Descriptor;
  UINTN                  MemoryMapSize;
  UINTN                  MemoryMapKey;
  UINTN                  DescriptorSize;
  UINT32                 DescriptorVersion;
  UINTN                  DescriptorCount;
  UINTN                  DescriptorIndex;

  (VOID)ImageHandle;
  (VOID)SystemTable;

  MemoryMap = NULL;
  MemoryMapSize = 0;
  MemoryMapKey = 0;
  DescriptorSize = 0;
  DescriptorVersion = 0;

  Status = gBS->GetMemoryMap(
                  &MemoryMapSize,
                  MemoryMap,
                  &MemoryMapKey,
                  &DescriptorSize,
                  &DescriptorVersion
                  );
  if (Status != EFI_BUFFER_TOO_SMALL || DescriptorSize == 0) {
    DEBUG((EFI_D_ERROR, "MUNCH_MEMMAP_ERROR probe status=%r\n", Status));
    return Status;
  }

  MemoryMapSize += 8 * DescriptorSize;
  Status = gBS->AllocatePool(EfiBootServicesData, MemoryMapSize, (VOID **)&MemoryMap);
  if (EFI_ERROR(Status)) {
    DEBUG((EFI_D_ERROR, "MUNCH_MEMMAP_ERROR allocate status=%r\n", Status));
    return Status;
  }

  Status = gBS->GetMemoryMap(
                  &MemoryMapSize,
                  MemoryMap,
                  &MemoryMapKey,
                  &DescriptorSize,
                  &DescriptorVersion
                  );
  if (EFI_ERROR(Status)) {
    DEBUG((EFI_D_ERROR, "MUNCH_MEMMAP_ERROR read status=%r\n", Status));
    gBS->FreePool(MemoryMap);
    return Status;
  }

  DescriptorCount = MemoryMapSize / DescriptorSize;
  DEBUG((
    EFI_D_INFO,
    "MUNCH_MEMMAP_BEGIN size=0x%lx desc=0x%lx ver=0x%x count=%lu key=0x%lx\n",
    MemoryMapSize,
    DescriptorSize,
    DescriptorVersion,
    DescriptorCount,
    MemoryMapKey
    ));

  for (DescriptorIndex = 0; DescriptorIndex < DescriptorCount; DescriptorIndex++) {
    Descriptor = (EFI_MEMORY_DESCRIPTOR *)((UINT8 *)MemoryMap + (DescriptorIndex * DescriptorSize));
    DEBUG((
      EFI_D_INFO,
      "MUNCH_MEMMAP type=%u phys=0x%016lx pages=0x%016lx attr=0x%016lx\n",
      Descriptor->Type,
      Descriptor->PhysicalStart,
      Descriptor->NumberOfPages,
      Descriptor->Attribute
      ));
  }

  DEBUG((EFI_D_INFO, "MUNCH_MEMMAP_END\n"));
  gBS->FreePool(MemoryMap);
  return EFI_SUCCESS;
}
