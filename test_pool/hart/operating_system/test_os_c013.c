/** @file
 * Copyright (c) 2016-2018, 2021, 2023 Arm Limited or its affiliates. All rights reserved.
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
#include "val/include/bsa_acs_hart.h"

#define TEST_NUM   (ACS_PE_TEST_NUM_BASE  +  13)
#define TEST_RULE  "B_PE_13"
#define TEST_DESC  "Check CRC32 instruction support       "

static
void
payload()
{
  uint64_t data = 0;
  uint32_t index = val_hart_get_index_mpid(val_hart_get_mpid());

  data = val_hart_reg_read(ID_AA64ISAR0_EL1);

  if ((data >> 16) & 0xF) //bits 19:16 are CRC32 and must not be zero
        val_set_status(index, RESULT_PASS(TEST_NUM, 1));
  else
        val_set_status(index, RESULT_FAIL(TEST_NUM, 1));

  return;

}

uint32_t
os_c013_entry(uint32_t num_hart)
{

  uint32_t status = ACS_STATUS_FAIL;

  status = val_initialize_test(TEST_NUM, TEST_DESC, num_hart);

  if (status != ACS_STATUS_SKIP)
      val_run_test_payload(TEST_NUM, num_hart, payload, 0);

  /* get the result from all HART and check for failure */
  status = val_check_for_error(TEST_NUM, num_hart, TEST_RULE);

  val_report_status(0, BSA_ACS_END(TEST_NUM), NULL);

  return status;
}
