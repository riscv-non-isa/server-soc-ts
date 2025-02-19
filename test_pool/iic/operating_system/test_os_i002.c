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
#include "val/include/bsa_acs_memory.h"

#include <sbi/riscv_asm.h>
#include <sbi/riscv_encoding.h>

#define TEST_NUM   (ACS_GIC_TEST_NUM_BASE + 2)
#define TEST_RULE  "MF_IIC_030_010"
#define TEST_DESC  "External interrupt functionality                     "

#define TEST_EXT_IRQ_ID   2

#define IMSIC_TOPEI_ID_SHIFT		16

/* Supervisor Indirect CSR Access */
#define CSR_SISELECT           0x150
#define CSR_SIREG                  0x151
/* Supervisor Interrupts */
#define CSR_STOPEI          0x15C
#define CSR_STOPI                  0xDB0

/**
 * @brief 1. Verify presence of siselect, sireg, stopi, and stopei CSRs.
          2. For each external interrupt identity supported by the S-level interrupt
          file, verify the ability to set the corresponding bit in the eipk and EIEk
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
          test by clearing the corresponding bits in the eipk and EIEk registers.
 */
static
void
payload()
{
  uint32_t index = val_hart_get_index_mpid(val_hart_get_mpid());
  uint64_t imsic_base;
  uint32_t intr_num = val_gic_max_supervisor_intr_num();
  uint32_t eidelivery;
  uint32_t i;
  uint64_t val;

  /* Checkpoint 1: Verify presence of siselect, sireg, stopi, and stopei CSRs. */
  val_print(ACS_PRINT_INFO, "\n       CSR_SISELECT: 0x%lx", csr_read(CSR_SISELECT));

  /* Try read siselect value 0x30 (major interrupt priorities) to see if sireg works */
  csr_write(CSR_SISELECT, 0x30);
  val_print(ACS_PRINT_INFO, "\n       CSR_SIREG: 0x%lx", csr_read(CSR_SIREG));
  val_print(ACS_PRINT_INFO, "\n       CSR_STOPEI: 0x%lx", csr_read(CSR_STOPEI));
  val_print(ACS_PRINT_INFO, "\n       CSR_STOPI: 0x%lx", csr_read(CSR_STOPI));

  /* Checkpoint 2: For each external interrupt identity supported by the S-level interrupt
      file, verify the ability to set the corresponding bit in the eipk and EIEk
      registers.
  */
  val_print(ACS_PRINT_INFO, "\n       S-level interrupt number: %d", intr_num);

  // for (i = 1; i <= intr_num; i++) {    // TODO: QEMU Virt problem? EIEk bit 32~63 read as 0 after set
  for (i = 1; i <= 31; i++) {
    /* Check EIEk */
    // val_print(ACS_PRINT_INFO, "\n         Set EIEk for irq %d", i);
    val_iic_imsic_eix_update(i, false, 1);
    val = val_iic_imsic_eix_read(i, false);
    // val_print(ACS_PRINT_INFO, "\n         Now EIEk is 0x%lx", val);
    if ((val & BIT(i % __riscv_xlen)) == 0) {
      val_print(ACS_PRINT_ERR, "\n       Fail to set EIEk for irq %d", i);
      val_set_status(index, RESULT_FAIL(TEST_NUM, 1));
      return;
    }
    // val_print(ACS_PRINT_INFO, "\n         Clear EIEk for irq %d", i);
    val_iic_imsic_eix_update(i, false, 0);

    /* Check eipk */
    // val_print(ACS_PRINT_INFO, "\n         Set EIPk for irq %d", i);
    val_iic_imsic_eix_update(i, true, 1);
    val = val_iic_imsic_eix_read(i, true);
    // val_print(ACS_PRINT_INFO, "\n         Now EIPk is 0x%lx", val);
    if ((val & BIT(i % __riscv_xlen)) == 0) {
      val_print(ACS_PRINT_ERR, "\n       Fail to set EIPk for irq %d", i);
      val_set_status(index, RESULT_FAIL(TEST_NUM, 2));
      return;
    }
    // val_print(ACS_PRINT_INFO, "\n         Clear EIPk for irq %d", i);
    val_iic_imsic_eix_update(i, true, 0);
  }

  /* Checkpoint 3: Verify ability to enable and disable interrupt delivery in the eidelivery
      register.
  */
  val_print(ACS_PRINT_INFO, "\n       Check EIDELIVERY enable/disable", 0);
  eidelivery = val_iic_imsic_eidelivery_update(0);
  val_print(ACS_PRINT_INFO, "\n         EIDELIVERY set to 0x%x", eidelivery);
  eidelivery = val_iic_imsic_eidelivery_update(1);
  val_print(ACS_PRINT_INFO, "\n         EIDELIVERY set to 0x%x", eidelivery);

  /* Checkpoint 4: Map the physical address of the S-mode interrupt register file of the hart
      with a virtual address using PBMT set to IO.
      */
  imsic_base = val_hart_get_imsic_base(index);
  if (imsic_base == 0) {
    val_print(ACS_PRINT_ERR, "\n       IMSIC base not found", 0);
    val_set_status(index, RESULT_FAIL(TEST_NUM, 2));
    return;
  }
  val_print(ACS_PRINT_INFO, "\n       IMSIC base: 0x%lx", imsic_base);
  val_memory_map_add_mmio(imsic_base, 0x1000);

  /* Checkpoint 5: Write a supported external interrupt identity to the S-level interrupt
          register file using a 4-byte store to the seteipnum_le register using
          virtual address established in previous step.
  */
  /* Enable the external interrupt */
  val = val_iic_imsic_eithreshold_update(0);
  val_print(ACS_PRINT_INFO, "\n       IMSIC_EITHRESHOLD set to: 0x%lx", val);

  val_print(ACS_PRINT_INFO, "\n       Set EIEk to enable the test irq %d", TEST_EXT_IRQ_ID);
  val_iic_imsic_eix_update(TEST_EXT_IRQ_ID, false, 1);

  /* Write value 2 to seteipnum_le to set pending bit for interrupt 2 */
  val_print(ACS_PRINT_INFO, "\n       Write interrupt file to set pending bit EIPk", 0);
  val_mmio_write(imsic_base + IMSIC_MMIO_PAGE_LE, TEST_EXT_IRQ_ID);

  /* Checkpoint 6: Read the seteipnum_le register using a 4-byte load to verify it reads 0 */
  val = val_mmio_read(imsic_base + IMSIC_MMIO_PAGE_LE);
  if (val != 0) {
    val_print(ACS_PRINT_ERR, "\n       SETEIPNUM_LE read as 0x%lx", val);
    val_set_status(index, RESULT_FAIL(TEST_NUM, 3));
    return;
  }

  /* Checkpoint 7: Verify that the written external interrupt identity is recorded in the eipk
          register of the IMSIC.
  */
  val_print(ACS_PRINT_INFO, "\n       Read and check pending bit in EIPk is set", 0);
  val = val_iic_imsic_eix_read(TEST_EXT_IRQ_ID, true);
  val_print(ACS_PRINT_INFO, "\n         EIPk: 0x%x", val);
  if ((val & BIT(TEST_EXT_IRQ_ID % __riscv_xlen)) == 0) {
    val_print(ACS_PRINT_ERR, "\n       External interrupt id not recorded in EIPk", 0);
    val_set_status(index, RESULT_FAIL(TEST_NUM, 4));
    return;
  }

  /* Checkpoint 9: Read the stopei register to verify that the highest priority external
          interrupt identity is reported.
  */
  val_print(ACS_PRINT_INFO, "\n       Read and check CSR_STOPI has expected value", 0);
  val = csr_read(CSR_STOPEI);
  val_print(ACS_PRINT_INFO, "\n         CSR_STOPEI: 0x%lx", val);
  if (val != (TEST_EXT_IRQ_ID | (TEST_EXT_IRQ_ID << IMSIC_TOPEI_ID_SHIFT))) {
    val_print(ACS_PRINT_ERR, "\n       Unexpected CSR_STOPI value: 0x%lx", val);
    val_set_status(index, RESULT_FAIL(TEST_NUM, 5));
    return;
  }

  /* Checkpoint 10: Clear any external interrupts pended or enabled in the IMSIC by this
          test by clearing the corresponding bits in the eipk and EIEk registers.
  */
  val_iic_imsic_eix_update(TEST_EXT_IRQ_ID, false, 0);
  val_iic_imsic_eix_update(TEST_EXT_IRQ_ID, true, 0);

  val_set_status(0, RESULT_PASS(TEST_NUM, 1));
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
