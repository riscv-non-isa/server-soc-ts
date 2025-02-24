/** @file
 * Copyright (c) 2016-2018, 2021, 2023, Arm Limited or its affiliates. All rights reserved.
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

#include "val/include/bsa_acs_timer.h"

#define TEST_NUM   (ACS_TIMER_TEST_NUM_BASE + 2)
#define TEST_RULE  "ME_CTI_020_010"
#define TEST_DESC  "Arch Context Lost Flags          "

/**
 * @brief Locate the _LPI objects for each RISC-V application processor hart. If
          present, verify that the Architectural Context Lost Flags have bit 0 set to 0,
          indicating no loss of timer context for each supported low-power idle state.
 */
static
void
payload()
{
  val_print(ACS_PRINT_ERR, "\n       Skip - Need ACPI interpreter to invoke _LPI method", 0);
  val_set_status(0, RESULT_SKIP(TEST_NUM, 1));
}

uint32_t
os_t002_entry(uint32_t num_hart)
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
