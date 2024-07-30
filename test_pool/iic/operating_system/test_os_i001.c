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

#define TEST_NUM   (ACS_GIC_TEST_NUM_BASE + 1)
#define TEST_RULE  "ME_IIC_010_010"
#define TEST_DESC  "Check GIC version                     "

/**
 * @brief For each application processor hart:
 *
 * 1. Determine the ISA node in ACPI RHCT table for that hart.
 * 2. Parse the ISA string in the ISA node and verify that Ssaia extension is
 *    supported.
 * 3. Parse the RINTC structure in ACPI MADT tables to verify that the
 *    interrupt controller type for the hart is IMSIC.
 */
static
void
payload()
{

  char8_t *isa_string;
  char8_t *ptr;
  uint32_t index = val_pe_get_index_mpid(val_pe_get_mpid());
  uint64_t imsic_base;

  isa_string = val_pe_get_isa_string(index);

  ptr = val_strstr(isa_string, "ssaia");
  if (ptr == NULL) {
    val_print(ACS_PRINT_ERR, "\n       Ssaia not found", 0);
    val_set_status(index, RESULT_FAIL(TEST_NUM, 1));
    return;
  }

  imsic_base = val_pe_get_imsic_base(index);
  if (imsic_base == 0) {
    val_print(ACS_PRINT_ERR, "\n       IMSIC base not found", 0);
    val_set_status(index, RESULT_FAIL(TEST_NUM, 2));
    return;
  }

  val_set_status(index, RESULT_PASS(TEST_NUM, 1));
}

uint32_t
os_i001_entry(uint32_t num_pe)
{

  uint32_t status = ACS_STATUS_FAIL;

  num_pe = 1;  //This IIC test is run on single processor
  // TODO: test all processor

  status = val_initialize_test(TEST_NUM, TEST_DESC, num_pe);

  if (status != ACS_STATUS_SKIP)
      val_run_test_payload(TEST_NUM, num_pe, payload, 0);

  /* get the result from all PE and check for failure */
  status = val_check_for_error(TEST_NUM, num_pe, TEST_RULE);

  val_report_status(0, BSA_ACS_END(TEST_NUM), NULL);

  return status;
}
