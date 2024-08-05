/** @file
 * Copyright (c) 2016-2018,2021 Arm Limited or its affiliates. All rights reserved.
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

#include "val/include/bsa_acs_qos.h"

#define TEST_NUM   (ACS_QOS_TEST_NUM_BASE + 1)
#define TEST_RULE  "OE_QOS_010_010"
#define TEST_DESC  "Check Ssqosid extension               "

/**
 * @brief 1. Determine the ISA node in ACPI RHCT table for hart 0.
          2. Parse the ISA string in the ISA node and report in test output log if
          Ssqosid extension is supported.
          3. Determine if ACPI RQSC table is present and if present report support
          for CBQRI extension in test output log.
 */
static
void
payload()
{
  char8_t *isa_string;
  char8_t *ptr;
  uint32_t index = val_hart_get_index_mpid(val_hart_get_mpid());

  isa_string = val_hart_get_isa_string(index);

  ptr = val_strstr(isa_string, "ssqosid");
  if (ptr == NULL) {
    val_print(ACS_PRINT_ERR, "\n       Ssqosid not found", 0);
    val_set_status(index, RESULT_FAIL(TEST_NUM, 1));
    return;
  }

  /* TODO: check ACPI RQSC table and CBQRI extension */
  val_print(ACS_PRINT_INFO, "\n       TODO: check ACPI RQSC table and CBQRI extension", 0);

  val_set_status(index, RESULT_PASS(TEST_NUM, 1));
}

uint32_t
os_q001_entry(uint32_t num_hart)
{

  uint32_t status = ACS_STATUS_FAIL;

  num_hart = 1;  //This Timer test is run on single processor

  status = val_initialize_test(TEST_NUM, TEST_DESC, num_hart);
  if (status != ACS_STATUS_SKIP)
      val_run_test_payload(TEST_NUM, num_hart, payload, 0);

  /* get the result from all HART and check for failure */
  status = val_check_for_error(TEST_NUM, num_hart, TEST_RULE);

  val_report_status(0, BSA_ACS_END(TEST_NUM), NULL);

  return status;
}
