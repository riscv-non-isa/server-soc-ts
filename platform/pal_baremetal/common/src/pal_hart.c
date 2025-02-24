/** @file
 * Copyright (c) 2023, Arm Limited or its affiliates. All rights reserved.
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

#include "pal_common_support.h"
#include "pal_pcie_enum.h"
#include "platform_override_struct.h"


extern HART_INFO_TABLE platform_hart_cfg;
extern HART_INFO_TABLE *g_hart_info_table;
extern int32_t gPsciConduit;

uint8_t   *gSecondaryPeStack;
uint64_t  gMpidrMax;

#define SIZE_STACK_SECONDARY_PE  0x100          //256 bytes per core
#define UPDATE_AFF_MAX(src,dest,mask)  ((dest & mask) > (src & mask) ? (dest & mask) : (src & mask))

/**
  Conduits for service calls (SMC vs HVC).
**/
#define CONDUIT_SMC       0
#define CONDUIT_HVC       1
#define CONDUIT_NONE     -2

uint64_t
pal_get_madt_ptr();

void
ArmCallSmc (
   ARM_SMC_ARGS *Args,
   int32_t Conduit
  );


/**
  @brief   Return the base address of the region allocated for Stack use for the Secondary
           PEs.
  @param   None
  @return  base address of the Stack
**/
uint64_t
PalGetSecondaryStackBase()
{

  return (uint64_t)gSecondaryPeStack;
}

/**
  @brief   Returns the Max of each 8-bit Affinity fields in MPIDR.
  @param   None
  @return  Max MPIDR
**/
uint64_t
PalGetMaxMpidr()
{

  return gMpidrMax;
}

/**
  @brief  Allocate memory region for secondary HART stack use. SIZE of stack for each HART
          is a #define

  @param  Number of PEs

  @return  None
**/
void
PalAllocateSecondaryStack(uint64_t mpidr)
{

  uint32_t NumPe, Aff0, Aff1, Aff2, Aff3;

  Aff0 = ((mpidr & 0x00000000ff) >>  0);
  Aff1 = ((mpidr & 0x000000ff00) >>  8);
  Aff2 = ((mpidr & 0x0000ff0000) >> 16);
  Aff3 = ((mpidr & 0xff00000000) >> 32);

  NumPe = ((Aff3+1) * (Aff2+1) * (Aff1+1) * (Aff0+1));

  if (gSecondaryPeStack == NULL)
  {
      gSecondaryPeStack = pal_aligned_alloc(MEM_ALIGN_4K, NumPe * SIZE_STACK_SECONDARY_PE);
      if (gSecondaryPeStack == NULL){
          print(ACS_PRINT_ERR, "FATAL - Allocation for Secondary stack failed\n", 0);
      }
      pal_hart_data_cache_ops_by_va((uint64_t)&gSecondaryPeStack, CLEAN_AND_INVALIDATE);
  }
}

/**
  @brief  This API fills in the HART_INFO Table with information about the PEs in the
          system. This is achieved by parsing the ACPI - MADT table.

  @param  PeTable  - Address where the HART information needs to be filled.

  @return  None
**/
void
pal_hart_create_info_table(HART_INFO_TABLE *PeTable)
{
  uint64_t MpidrAff0Max = 0;
  uint64_t MpidrAff1Max = 0;
  uint64_t MpidrAff2Max = 0;
  uint64_t MpidrAff3Max = 0;
  uint32_t PeIndex = 0;

  if (PeTable == NULL) {
    return;
  }

  PeTable->header.num_of_hart = platform_hart_cfg.header.num_of_hart;
  if (PeTable->header.num_of_hart == 0) {
    return;
  }

  while (PeIndex < PeTable->header.num_of_hart) {

      PeTable->hart_info[PeIndex].mpidr = platform_hart_cfg.hart_info[PeIndex].mpidr;
      PeTable->hart_info[PeIndex].hart_num = PeIndex;
      PeTable->hart_info[PeIndex].pmu_gsiv = platform_hart_cfg.hart_info[PeIndex].pmu_gsiv;
      PeTable->hart_info[PeIndex].gmain_gsiv = platform_hart_cfg.hart_info[PeIndex].gmain_gsiv;
      pal_hart_data_cache_ops_by_va((uint64_t)(&PeTable->hart_info[PeIndex]), CLEAN_AND_INVALIDATE);

      MpidrAff0Max = UPDATE_AFF_MAX(MpidrAff0Max, PeTable->hart_info[PeIndex].mpidr, 0x00000000ff);
      MpidrAff1Max = UPDATE_AFF_MAX(MpidrAff1Max, PeTable->hart_info[PeIndex].mpidr, 0x000000ff00);
      MpidrAff2Max = UPDATE_AFF_MAX(MpidrAff2Max, PeTable->hart_info[PeIndex].mpidr, 0x0000ff0000);
      MpidrAff3Max = UPDATE_AFF_MAX(MpidrAff3Max, PeTable->hart_info[PeIndex].mpidr, 0xff00000000);

      PeIndex++;
  };

  gMpidrMax = MpidrAff0Max | MpidrAff1Max | MpidrAff2Max | MpidrAff3Max;
  pal_hart_data_cache_ops_by_va((uint64_t)PeTable, CLEAN_AND_INVALIDATE);
  pal_hart_data_cache_ops_by_va((uint64_t)&gMpidrMax, CLEAN_AND_INVALIDATE);
  PalAllocateSecondaryStack(gMpidrMax);

}

/**
  @brief  Make the SMC call using AARCH64 Assembly code
          SMC calls can take up to 7 arguments and return up to 4 return values.
          Therefore, the 4 first fields in the ARM_SMC_ARGS structure are used
          for both input and output values.

  @param  Argumets to pass to the EL3 firmware

  @return  None
**/
void
pal_hart_call_smc(ARM_SMC_ARGS *ArmSmcArgs, int32_t Conduit)
{

  if(ArmSmcArgs == NULL){
    return;
  }

  ArmCallSmc (ArmSmcArgs, Conduit);
}

void
ModuleEntryPoint();

/**
  @brief  Make a PSCI CPU_ON call using SMC instruction.
          Pass PAL Assembly code entry as the start vector for the PSCI ON call

  @param  Argumets to pass to the EL3 firmware

  @return  None
**/
void
pal_hart_execute_payload(ARM_SMC_ARGS *ArmSmcArgs)
{

  if(ArmSmcArgs == NULL){
     return;
  }

  ArmSmcArgs->Arg2 = (uint64_t)ModuleEntryPoint;
  pal_hart_call_smc(ArmSmcArgs, gPsciConduit);
}

void
DataCacheCleanInvalidateVA(uint64_t addr);

void
DataCacheCleanVA(uint64_t addr);

void
DataCacheInvalidateVA(uint64_t addr);

/**
  @brief Perform cache maintenance operation on an address

  @param addr - address on which cache ops to be performed
  @param type - type of cache ops

  @return  None
**/
void
pal_hart_data_cache_ops_by_va(uint64_t addr, uint32_t type)
{
  switch(type){
      case CLEAN_AND_INVALIDATE:
          DataCacheCleanInvalidateVA(addr);
      break;
      case CLEAN:
          DataCacheCleanVA(addr);
      break;
      case INVALIDATE:
          DataCacheInvalidateVA(addr);
      break;
      default:
          DataCacheCleanInvalidateVA(addr);
  }

}

/**
  @brief Returns the number of currently present PEs

  @return  The number of PEs that are present in the system
**/
uint32_t
pal_hart_get_num()
{
  if (g_hart_info_table == NULL) {
      return 0;
  }
  return g_hart_info_table->header.num_of_hart;
}
