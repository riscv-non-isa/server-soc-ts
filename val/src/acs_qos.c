/** @file
 * Copyright (c) 2016-2023, Arm Limited or its affiliates. All rights reserved.
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

#include "include/bsa_acs_val.h"
#include "include/bsa_acs_qos.h"
#include "include/bsa_acs_common.h"


/**
  @brief   Pointer to the memory location of the HART Information table
**/
extern HART_INFO_TABLE *g_hart_info_table;


/**
  @brief   This API will execute all QoS tests designated for a given compliance level
           1. Caller       -  Application layer.
           2. Prerequisite -  val_hart_create_info_table, val_allocate_shared_mem
  @param   num_hart - the number of HART to run these tests on.
  @param   g_sw_view - Keeps the information about which view tests to be run
  @return  Consolidated status of all the tests run.
**/
uint32_t
val_qos_execute_tests(uint32_t num_hart, uint32_t *g_sw_view)
{
  uint32_t status, i;

  for (i = 0; i < g_num_skip; i++) {
      if (g_skip_test_num[i] == ACS_QOS_TEST_NUM_BASE) {
        val_print(ACS_PRINT_INFO, "\n       USER Override - Skipping all QoS tests\n", 0);
        return ACS_STATUS_SKIP;
      }
  }

  /* Check if there are any tests to be executed in current module with user override options*/
  status = val_check_skip_module(ACS_QOS_TEST_NUM_BASE);
  if (status) {
      val_print(ACS_PRINT_INFO, "\n       USER Override - Skipping all QoS tests\n", 0);
      return ACS_STATUS_SKIP;
  }

  status = ACS_STATUS_PASS;

  val_print_test_start("QoS");
  g_curr_module = 1 << PE_MODULE;

  if (g_sw_view[G_SW_OS]) {
      val_print(ACS_PRINT_ERR, "\nOperating System View:\n", 0);
      status |= os_q001_entry(num_hart);
      // status |= os_c002_entry(num_hart);
      // status |= os_c003_entry(num_hart);
      // status |= os_c004_entry(num_hart);
  }

  if (g_sw_view[G_SW_HYP]) {
      val_print(ACS_PRINT_ERR, "\nHypervisor View:\n", 0);
      // status |= hyp_c001_entry(num_hart);
      // status |= hyp_c002_entry(num_hart);
      // status |= hyp_c003_entry(num_hart);
      // status |= hyp_c004_entry(num_hart);
      // status |= hyp_c005_entry(num_hart);
  }

  if (g_sw_view[G_SW_PS]) {
      val_print(ACS_PRINT_ERR, "\nPlatform Security View:\n", 0);
      // status |= ps_c001_entry(num_hart);
  }

  val_print_test_end(status, "QoS");

  return status;

}