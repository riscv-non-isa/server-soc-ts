/** @file
 * Copyright (c) 2019-2020, Arm Limited or its affiliates. All rights reserved.
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

#include "val/include/val_interface.h"
#include "test_os_p020_data.h"
#include "val/include/bsa_acs_val.h"

#define TEST_NUM   (ACS_PCIE_TEST_NUM_BASE + 20)
#define TEST_RULE  "PCI_IN_05, PCI_IN_19"
#define TEST_DESC  "Type 0/1 common config rule           "

static
void
payload(void)
{

  uint32_t hart_index;
  uint32_t ret;
  uint32_t table_entries;

  hart_index = val_hart_get_index_mpid(val_hart_get_mpid());

  table_entries = sizeof(bf_info_table20)/sizeof(bf_info_table20[0]);
  ret = val_pcie_register_bitfields_check((void *)&bf_info_table20, table_entries);

  if (ret == ACS_STATUS_SKIP)
      val_set_status(hart_index, RESULT_SKIP(TEST_NUM, 1));
  else if (ret)
      val_set_status(hart_index, RESULT_FAIL(TEST_NUM, ret));
  else
      val_set_status(hart_index, RESULT_PASS(TEST_NUM, 1));

}

uint32_t
os_p020_entry(uint32_t num_hart)
{

  uint32_t status = ACS_STATUS_FAIL;

  num_hart = 1;  //This test is run on single processor

  status = val_initialize_test(TEST_NUM, TEST_DESC, num_hart);
  if (status != ACS_STATUS_SKIP)
      val_run_test_payload(TEST_NUM, num_hart, payload, 0);

  /* get the result from all HART and check for failure */
  status = val_check_for_error(TEST_NUM, num_hart, TEST_RULE);

  val_report_status(0, BSA_ACS_END(TEST_NUM), NULL);

  return status;
}
