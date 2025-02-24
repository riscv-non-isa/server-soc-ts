/** @file
 * Copyright (c) 2016-2018, 2020, 2021, 2023, Arm Limited or its affiliates. All rights reserved.
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
#include <Uefi.h>
#include <Library/UefiLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/ShellLib.h>

#include <IndustryStandard/SmBios.h>
#include <Protocol/Smbios.h>

#include "include/pal_uefi.h"

/**
  @brief  This API fills in the MNG_INFO_TABLE with information about manageability
          in the system. This is achieved by parsing the SMBIOS table.

  @param  TimerTable  - Address where the MNG information needs to be filled.

  @return  None
**/
VOID
pal_mng_create_info_table(MNG_INFO_TABLE *MngTable)
{
  EFI_STATUS                Status;
  EFI_SMBIOS_PROTOCOL       *Smbios;
  EFI_SMBIOS_HANDLE         Handle;
  SMBIOS_STRUCTURE_POINTER  SmbiosTable;

  if (MngTable == NULL) {
    bsa_print(ACS_PRINT_ERR, L" Input MNG Table Pointer is NULL. Cannot create MNG INFO\n");
    return;
  }

  ZeroMem(MngTable, sizeof (MNG_INFO_TABLE));

  Status = gBS->LocateProtocol (
                  &gEfiSmbiosProtocolGuid,
                  NULL,
                  (VOID **)&Smbios
                  );
  if (EFI_ERROR (Status)) {
    bsa_print(ACS_PRINT_ERR, L" Can't find SMBIOS protocol\n");
    return;
  }

  Handle = SMBIOS_HANDLE_PI_RESERVED;
  for (Status = Smbios->GetNext (Smbios, &Handle, NULL, &SmbiosTable.Hdr, NULL);
       !EFI_ERROR (Status);
       Status = Smbios->GetNext (Smbios, &Handle, NULL, &SmbiosTable.Hdr, NULL))
  {
    bsa_print(ACS_PRINT_INFO, L"   Find SMBIOS Type %d at 0x%lx\n", SmbiosTable.Hdr->Type, (UINTN)SmbiosTable.Hdr);
    if (SmbiosTable.Hdr->Type == SMBIOS_TYPE_MANAGEMENT_CONTROLLER_HOST_INTERFACE) {
      MngTable->mc_host_if_tbl_addr = (UINTN)SmbiosTable.Hdr;
      MngTable->mc_host_if_type = SmbiosTable.Type42->InterfaceType;
    } else if (SmbiosTable.Hdr->Type == SMBIOS_TYPE_IPMI_DEVICE_INFORMATION) {
      MngTable->ipmi_device_info_tbl_addr = (UINTN)SmbiosTable.Hdr;
      MngTable->ipmi_device_if_type = SmbiosTable.Type38->InterfaceType;
    }
  }
}