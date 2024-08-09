/** @file
 * Copyright (c) 2016-2018, 2021, Arm Limited or its affiliates. All rights reserved.
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

#include "val/include/bsa_acs_val.h"
#include "val/include/val_interface.h"

#include "val/include/bsa_acs_gic.h"
#include "val/include/bsa_acs_iic.h"
#include "val/include/bsa_acs_hart.h"

#include <sbi/riscv_asm.h>
#include <sbi/riscv_encoding.h>

#define TEST_NUM   (ACS_GIC_TEST_NUM_BASE + 2)
#define TEST_RULE  "MF_IIC_030_010"
#define TEST_DESC  "External interrupt functionality                     "

/**
 * @brief 1. Verify presence of siselect, sireg, stopi, and stopei CSRs.
          2. For each external interrupt identity supported by the S-level interrupt
          file, verify the ability to set the corresponding bit in the eipk and eiek
          registers.
          3. Verify ability to enable and disable interrupt delivery in the eidelivery
          register.
          4. Map the physical address of the S-mode interrupt register file of the hart
          with a virtual address using PBMT set to IO. The physical address is
          provided by the RINTC structure in ACPI MADT table.
          5. Write a supported external interrupt identity to the S-level interrupt
          register file using a 4-byte store to the seteipnum_le register using
          virtual address established in previous step.
          6. Read the seteipnum_le register using a 4-byte load to verify it reads 0.
          7. Verify that the written external interrupt identity is recorded in the eipk
          register of the IMSIC.
          8. Determine the highest priority pending and enabled interrupt in the
          eipk registers.
          9. Read the stopei register to verify that the highest priority external
          interrupt identity is reported.
          10. Clear any external interrupts pended or enabled in the IMSIC by this
          test by clearing the corresponding bits in the eipk and eiek registers.
 */
static
void
payload()
{
  /* Check point 1: Verify presence of siselect, sireg, stopi, and stopei CSRs. */
  val_print(ACS_PRINT_ERR, "\n       CSR_SISELECT: 0x%lx", csr_read(CSR_SISELECT));

  /*
    The allocated values for siselect:
      0x00–0x2F reserved
      0x30–0x3F major interrupt priorities
      0x40–0x6F reserved
      0x70–0xFF external interrupts (only with an IMSIC)
  */
  /* Try read siselect value 0x30 to see if sireg works */
  csr_write(CSR_SISELECT, 0x30);
  val_print(ACS_PRINT_ERR, "\n       CSR_SIREG: 0x%lx", csr_read(CSR_SIREG));
  val_print(ACS_PRINT_ERR, "\n       CSR_STOPEI: 0x%lx", csr_read(CSR_STOPEI));
  val_print(ACS_PRINT_ERR, "\n       CSR_STOPI: 0x%lx", csr_read(CSR_STOPI));

  val_print(ACS_PRINT_ERR, "\n       Skip - rest not implemented", 0);
  val_set_status(0, RESULT_SKIP(TEST_NUM, 1));
}

uint32_t
os_i002_entry(uint32_t num_hart)
{

  uint32_t status = ACS_STATUS_FAIL;

  num_hart = 1;  //This IIC test is run on single processor
  // TODO: test all processor

  status = val_initialize_test(TEST_NUM, TEST_DESC, num_hart);

  if (status != ACS_STATUS_SKIP)
      val_run_test_payload(TEST_NUM, num_hart, payload, 0);

  /* get the result from all HART and check for failure */
  status = val_check_for_error(TEST_NUM, num_hart, TEST_RULE);

  val_report_status(0, BSA_ACS_END(TEST_NUM), NULL);

  return status;
}
