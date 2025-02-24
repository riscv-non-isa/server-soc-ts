#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiLib.h>

#include "Include/IndustryStandard/Acpi65.h"
#include <Protocol/AcpiTable.h>

#include "include/pal_uefi.h"

static EFI_ACPI_6_5_RISC_V_IO_MAPPING_TABLE_STRUCTURE *gRimtHdr;

UINT64
pal_get_rimt_ptr();

/**
  @brief  Populate information about the IOMMU sub-system at the input address.
          In a UEFI-ACPI framework, this information is part of the RIMT table.

  @param  GicTable  Address of the memory region where this information is to be filled in

  @return None
**/
VOID
pal_iommu_create_info_table(IOMMU_INFO_TABLE *IommuTable)
{
  UINT32                            TableLength;
  EFI_ACPI_6_5_RIMT_DEVICE_HEADER   *Entry;
  EFI_ACPI_6_5_RIMT_IOMMU_DEVICE_STRUCTURE  *IommuEntry;
  IOMMU_INFO_ENTRY                  *Ptr = NULL;

  UINT32                            Length = 0;

  bsa_print(ACS_PRINT_INFO, L" Creating IOMMU INFO\n");

  if (IommuTable == NULL) {
    bsa_print(ACS_PRINT_ERR, L" Input IOMMU Table Pointer is NULL. Cannot create IOMMU INFO\n");
    return;
  }

  gRimtHdr = (EFI_ACPI_6_5_RISC_V_IO_MAPPING_TABLE_STRUCTURE *) pal_get_rimt_ptr();

  if (gRimtHdr != NULL) {
    TableLength =  gRimtHdr->Header.Length;
    bsa_print(ACS_PRINT_INFO, L"  RIMT table is at %x and length is 0x%x\n", gRimtHdr, TableLength);
  } else {
    bsa_print(ACS_PRINT_ERR, L" RIMT not found\n");
    return;
  }

  IommuTable->header.num_of_iommu = 0;
  Entry = (EFI_ACPI_6_5_RIMT_DEVICE_HEADER *) ((UINT8 *)gRimtHdr + gRimtHdr->RimtDeviceOffset);
  Length = gRimtHdr->RimtDeviceOffset;
  Ptr = IommuTable->iommu_info;


  do {
    Ptr->iommu_num = IommuTable->header.num_of_iommu;
    Ptr->type = Entry->Type;
    Ptr->id = Entry->ID;
    bsa_print(ACS_PRINT_INFO, L"   RIMT device id %d, type %d\n", Ptr->id, Ptr->type);


    if (Entry->Type == EFI_ACPI_6_5_RIMT_DEVICE_TYPE_IOMMU) {
      IommuEntry = (EFI_ACPI_6_5_RIMT_IOMMU_DEVICE_STRUCTURE *) Entry;
      Ptr->hardware_id  = IommuEntry->HardwareID;
      Ptr->base_address = IommuEntry->BaseAddress;
      Ptr->flags        = IommuEntry->Flags;
      bsa_print(ACS_PRINT_INFO, L"    IOMMU base address 0x%lx\n", Ptr->base_address);
    }

    Length += Entry->Length;
    Entry = (EFI_ACPI_6_5_RIMT_DEVICE_HEADER *) ((UINT8 *)Entry + (Entry->Length));
    Ptr++;
    IommuTable->header.num_of_iommu++;
  } while (Length < TableLength);

  if (IommuTable->header.num_of_iommu != gRimtHdr->RimtDeviceNumber) {
    bsa_print(ACS_PRINT_ERR, L" RIMT device number mismatch!\n");
  }
}