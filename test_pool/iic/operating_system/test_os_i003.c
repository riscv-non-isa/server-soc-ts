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
#include "val/include/bsa_acs_pe.h"

#define TEST_NUM   (ACS_GIC_TEST_NUM_BASE + 3)
#define TEST_RULE  "ME_IIC_040_010"
#define TEST_DESC  "Check maximum hstatus.VGEIN                     "

#define HSTATUS_VGEIN_SHIFT		12
#define HSTATUS_VGEIN			0x0003f000UL

/**
 * @brief Use WARL discovery method on hstatus.VGEIN CSR field to determine the
          GEILEN and verify that at least 5 guest interrupt files are supported
 */
static
void
payload()
{
  uint32_t index = val_pe_get_index_mpid(val_pe_get_mpid());
  uint32_t vgein;
  uint64_t val_w;
  uint64_t val_r;

  for (vgein = 1; vgein <= 0x3F; vgein++) {
    val_w = (val_pe_get_hstatus() & (~HSTATUS_VGEIN)) | (vgein << HSTATUS_VGEIN_SHIFT);
    val_pe_set_hstatus(val_w);
    val_r = val_pe_get_hstatus();
    // val_print(ACS_PRINT_INFO, "\n       val_w=0x%lx", val_w);
    // val_print(ACS_PRINT_INFO, "\n       val_r=0x%lx", val_r);
    if ((val_w & HSTATUS_VGEIN) != (val_r & HSTATUS_VGEIN)) {
      break;
    }
  }

  val_print(ACS_PRINT_INFO, "\n       Valid VGEIN range is [0, 0x%x)", vgein);

  if (vgein < 5) {
    val_print(ACS_PRINT_ERR, "\n       GEILEN must be at least 5", 0);
    val_set_status(index, RESULT_FAIL(TEST_NUM, 1));
    return;
  }

  val_set_status(index, RESULT_PASS(TEST_NUM, 1));
}

uint32_t
os_i003_entry(uint32_t num_pe)
{

  uint32_t status = ACS_STATUS_FAIL;

  num_pe = 1;  //This IIC test is run on single processor

  status = val_initialize_test(TEST_NUM, TEST_DESC, num_pe);

  if (status != ACS_STATUS_SKIP)
      val_run_test_payload(TEST_NUM, num_pe, payload, 0);

  /* get the result from all PE and check for failure */
  status = val_check_for_error(TEST_NUM, num_pe, TEST_RULE);

  val_report_status(0, BSA_ACS_END(TEST_NUM), NULL);

  return status;
}
