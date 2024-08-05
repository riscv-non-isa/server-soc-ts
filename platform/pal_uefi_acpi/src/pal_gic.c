/** @file
 * Copyright (c) 2016-2021, 2023, Arm Limited or its affiliates. All rights reserved.
 * SPDX-License-Identifier : Apache-2.0

 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
**/

#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiLib.h>

#include "Include/IndustryStandard/Acpi61.h"
#include <Protocol/AcpiTable.h>
#include <Protocol/HardwareInterrupt.h>
#include <Protocol/HardwareInterrupt2.h>

#include "include/pal_uefi.h"
#include "include/bsa_pcie_enum.h"

static EFI_ACPI_6_1_MULTIPLE_APIC_DESCRIPTION_TABLE_HEADER *gMadtHdr;

EFI_HARDWARE_INTERRUPT_PROTOCOL *gInterrupt = NULL;
EFI_HARDWARE_INTERRUPT2_PROTOCOL *gInterrupt2 = NULL;

UINT64
pal_get_madt_ptr();

/**
  @brief  Populate information about the IIC sub-system at the input address.
          In a UEFI-ACPI framework, this information is part of the MADT table.

  @param  GicTable  Address of the memory region where this information is to be filled in

  @return None
**/
VOID
pal_gic_create_info_table(GIC_INFO_TABLE *GicTable)
{
  EFI_ACPI_6_1_GIC_STRUCTURE    *Entry = NULL;
  EFI_ACPI_6_5_IMSIC_STRUCTURE  *ImsicEntry = NULL;
  GIC_INFO_ENTRY                *GicEntry = NULL;
  UINT32                         Length= 0;
  UINT32                         TableLength;
  // UINT32                         is_gicr_present = 0;

  if (GicTable == NULL) {
    bsa_print(ACS_PRINT_ERR, L" Input IIC Table Pointer is NULL. Cannot create IIC INFO\n");
    return;
  }

  GicEntry = GicTable->gic_info;

  GicTable->header.gic_version = 0;
  GicTable->header.num_gicr_rd = 0;
  GicTable->header.num_gicc_rd = 0;
  GicTable->header.num_gicd = 0;
  GicTable->header.num_its = 0;
  GicTable->header.num_msi_frame = 0;

  gMadtHdr = (EFI_ACPI_6_1_MULTIPLE_APIC_DESCRIPTION_TABLE_HEADER *) pal_get_madt_ptr();

  if (gMadtHdr != NULL) {
    TableLength =  gMadtHdr->Header.Length;
    bsa_print(ACS_PRINT_INFO, L"  MADT is at %x and length is %x\n", gMadtHdr, TableLength);
  } else {
    bsa_print(ACS_PRINT_ERR, L" MADT not found\n");
    return;
  }

  // Entry = (EFI_ACPI_6_1_GIC_STRUCTURE *) (gMadtHdr + 1);
  // Length = sizeof (EFI_ACPI_6_1_MULTIPLE_APIC_DESCRIPTION_TABLE_HEADER);

  // /* Check if GICR Structure is present */
  // do {

  //   if (Entry->Type == EFI_ACPI_6_1_GICR) {
  //       is_gicr_present = 1;
  //       break;
  //   }

  //   Length += Entry->Length;
  //   Entry = (EFI_ACPI_6_1_GIC_STRUCTURE *) ((UINT8 *)Entry + (Entry->Length));

  // } while(Length < TableLength);

  Entry = (EFI_ACPI_6_1_GIC_STRUCTURE *) (gMadtHdr + 1);
  Length = sizeof (EFI_ACPI_6_1_MULTIPLE_APIC_DESCRIPTION_TABLE_HEADER);

  do {
    // The IMSIC structure present in the MADT is to provide information common across processors.
    if (Entry->Type == EFI_ACPI_6_5_IMSIC) {
      ImsicEntry = (EFI_ACPI_6_5_IMSIC_STRUCTURE *) Entry;
      bsa_print(ACS_PRINT_INFO, L"   RISC-V IMSIC is found\n");
      GicTable->header.supervisor_intr_num = ImsicEntry->SupervisorModeInterruptIdentityNumber;
      GicTable->header.guest_intr_num = ImsicEntry->GuestModeInterruptIdentityNumber;
    }

    if (Entry->Type == EFI_ACPI_6_5_PLIC) {
      GicEntry->type = ENTRY_TYPE_PLIC;
      bsa_print(ACS_PRINT_INFO, L"   RISC-V PLIC is found, record field TBD\n");
    }

    Length += Entry->Length;
    Entry = (EFI_ACPI_6_1_GIC_STRUCTURE *) ((UINT8 *)Entry + (Entry->Length));
  } while(Length < TableLength);

  GicEntry->type = 0xFF;  //Indicate end of data

}

/**
  @brief  Enable the interrupt in the IIC Distributor and IIC CPU Interface and hook
          the interrupt service routine for the IRQ to the UEFI Framework

  @param  int_id  Interrupt ID which needs to be enabled and service routine installed for
  @param  isr     Function pointer of the Interrupt service routine

  @return Status of the operation
**/
UINT32
pal_gic_install_isr(UINT32 int_id,  VOID (*isr)())
{

  EFI_STATUS  Status;

 // Find the interrupt controller protocol.
  Status = gBS->LocateProtocol (&gHardwareInterruptProtocolGuid, NULL, (VOID **)&gInterrupt);
  if (EFI_ERROR(Status)) {
    return 0xFFFFFFFF;
  }

  //First disable the interrupt to enable a clean handoff to our Interrupt handler.
  gInterrupt->DisableInterruptSource(gInterrupt, int_id);

  //Register our handler
  Status = gInterrupt->RegisterInterruptSource (gInterrupt, int_id, isr);
  if (EFI_ERROR(Status)) {
    Status =  gInterrupt->RegisterInterruptSource (gInterrupt, int_id, NULL);  //Deregister existing handler
    Status = gInterrupt->RegisterInterruptSource (gInterrupt, int_id, isr);  //register our Handler.
    //Even if this fails. there is nothing we can do in UEFI mode
  }

  return 0;
}

/**
  @brief  Indicate that processing of interrupt is complete by writing to
          End of interrupt register in the IIC CPU Interface

  @param  int_id  Interrupt ID which needs to be acknowledged that it is complete

  @return Status of the operation
**/
UINT32
pal_gic_end_of_interrupt(UINT32 int_id)
{

  EFI_STATUS  Status;

 // Find the interrupt controller protocol.
  Status = gBS->LocateProtocol (&gHardwareInterruptProtocolGuid, NULL, (VOID **)&gInterrupt);
  if (EFI_ERROR(Status)) {
    return 0xFFFFFFFF;
  }

  //EndOfInterrupt.
  gInterrupt->EndOfInterrupt(gInterrupt, int_id);

  return 0;
}

/**
  @brief  Set Trigger type Edge/Level

  @param  int_id  Interrupt ID which needs to be enabled and service routine installed for
  @param  trigger_type  Interrupt Trigger Type Edge/Trigger

  @return Status of the operation
**/
UINT32
pal_gic_set_intr_trigger(UINT32 int_id, INTR_TRIGGER_INFO_TYPE_e trigger_type)
{

  EFI_STATUS  Status;

  /* Find the interrupt protocol. */
  Status = gBS->LocateProtocol (&gHardwareInterrupt2ProtocolGuid, NULL, (VOID **)&gInterrupt2);
  if (EFI_ERROR(Status)) {
    return 0xFFFFFFFF;
  }

  Status = gInterrupt2->SetTriggerType (
                   gInterrupt2,
                   int_id,
                   (EFI_HARDWARE_INTERRUPT2_TRIGGER_TYPE)trigger_type
                   );

  if (EFI_ERROR(Status))
    return 0xFFFFFFFF;

  return 0;
}

/** Place holder function. Need to be implemented if needed in later releases
  @brief Registers the interrupt handler for a given IRQ

  @param IrqNum Hardware IRQ number
  @param MappedIrqNum Mapped IRQ number
  @param Isr Interrupt Service Routine that returns the status

  @return Status of the operation
**/
UINT32
pal_gic_request_irq(
  UINT32 IrqNum,
  UINT32 MappedIrqNum,
  VOID *Isr
  )
{
    return 0;
}

/** Place holder function. Need to be implemented if needed in later releases
  @brief This function frees the registered interrupt handler for a given IRQ

  @param IrqNum Hardware IRQ number
  @param MappedIrqNum Mapped IRQ number

  @return none
**/
VOID
pal_gic_free_irq(
  UINT32 IrqNum,
  UINT32 MappedIrqNum
  )
{

}
