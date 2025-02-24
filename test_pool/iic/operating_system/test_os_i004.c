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

#define TEST_NUM   (ACS_GIC_TEST_NUM_BASE + 4)
#define TEST_RULE  "ME_IIC_050_010"
#define TEST_DESC  "Check supported supervisor mode interrupt identities          "

/**
 * @brief Verify the number of supported supervisor mode interrupt identities in
          IMSIC structure of the ACPI MADT table is at least 255
 */
static
void
payload()
{
  uint32_t index = val_hart_get_index_mpid(val_hart_get_mpid());

  if (val_gic_max_supervisor_intr_num() < 255) {
    val_print(ACS_PRINT_ERR, "\n       Supported supervisor mode interrupt identities must be at least 255", 0);
    val_set_status(index, RESULT_FAIL(TEST_NUM, 1));
    return;
  }

  val_set_status(index, RESULT_PASS(TEST_NUM, 1));
}

uint32_t
os_i004_entry(uint32_t num_hart)
{

  uint32_t status = ACS_STATUS_FAIL;

  num_hart = 1;  //This IIC test is run on single processor

  status = val_initialize_test(TEST_NUM, TEST_DESC, num_hart);

  if (status != ACS_STATUS_SKIP)
      val_run_test_payload(TEST_NUM, num_hart, payload, 0);

  /* get the result from all HART and check for failure */
  status = val_check_for_error(TEST_NUM, num_hart, TEST_RULE);

  val_report_status(0, BSA_ACS_END(TEST_NUM), NULL);

  return status;
}
