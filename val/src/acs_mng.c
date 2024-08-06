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
#include "include/bsa_acs_mng.h"
#include "include/bsa_acs_common.h"


/**
  @brief   Pointer to the memory location of the MNG Information table
**/
MNG_INFO_TABLE *g_mng_info_table;

/**
  @brief   This API will call PAL layer to fill in the MNG information
           into the g_mng_info_table pointer.
           1. Caller       -  Application layer.
           2. Prerequisite -  Memory allocated and passed as argument.
  @param   mng_info_table  pre-allocated memory pointer for mng_info
  @return  Error if Input param is NULL.
**/
void
val_mng_create_info_table(uint64_t *mng_info_table)
{
  if (mng_info_table == NULL) {
      val_print(ACS_PRINT_ERR, "Input for Create Info table cannot be NULL\n", 0);
      return;
  }
  val_print(ACS_PRINT_INFO, "\n Creating MNG INFO table\n", 0);

  g_mng_info_table = (MNG_INFO_TABLE *)mng_info_table;

  pal_mng_create_info_table(g_mng_info_table);

  val_print(ACS_PRINT_TEST, " MNG_INFO table created\n", 0);

}

/**
  @brief   This API will execute all MNG tests designated for a given compliance level
           1. Caller       -  Application layer.
           2. Prerequisite -  val_mng_create_info_table, val_allocate_shared_mem
  @param   num_hart - the number of HART to run these tests on.
  @param   g_sw_view - Keeps the information about which view tests to be run
  @return  Consolidated status of all the tests run.
**/
uint32_t
val_mng_execute_tests(uint32_t num_hart, uint32_t *g_sw_view)
{
  uint32_t status, i;

  for (i = 0; i < g_num_skip; i++) {
      if (g_skip_test_num[i] == ACS_MNG_TEST_NUM_BASE) {
        val_print(ACS_PRINT_INFO, "\n       USER Override - Skipping all MNG tests\n", 0);
        return ACS_STATUS_SKIP;
      }
  }

  /* Check if there are any tests to be executed in current module with user override options*/
  status = val_check_skip_module(ACS_MNG_TEST_NUM_BASE);
  if (status) {
      val_print(ACS_PRINT_INFO, "\n       USER Override - Skipping all MNG tests\n", 0);
      return ACS_STATUS_SKIP;
  }

  status = ACS_STATUS_PASS;

  val_print_test_start("MNG");
  g_curr_module = 1 << PE_MODULE;

  if (g_sw_view[G_SW_OS]) {
      val_print(ACS_PRINT_ERR, "\nOperating System View:\n", 0);
      status |= os_m001_entry(num_hart);
      status |= os_m002_entry(num_hart);
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

  val_print_test_end(status, "MNG");

  return status;

}

/**
  @brief   This function is a single point of entry to retrieve
           all MNG related information.
           1. Caller       -  Test Suite
           2. Prerequisite -  val_mng_create_info_table
  @param   type   the type of information being requested
  @return  32-bit data
**/
uint32_t
val_mng_get_info(MNG_INFO_e type)
{

  if (g_mng_info_table == NULL) {
      val_print(ACS_PRINT_ERR, "\n   Get MNG info called before mng info table is filled ",        0);
      return 0;
  }

  switch (type) {
    case MNG_MC_HOST_IF_TABLE:
      return g_mng_info_table->mc_host_if_tbl_addr;

    case MNG_MC_DEVICE_TYPE:
      return g_mng_info_table->mc_host_if_type;

    case MNG_IPMI_DEVICE_INFO_TABLE:
      return g_mng_info_table->ipmi_device_info_tbl_addr;

    case MNG_IPMI_IF_TYPE:
      return g_mng_info_table->ipmi_device_if_type;

    default:
      val_print(ACS_PRINT_ERR, "\n    IIC Info - TYPE not recognized %d  ", type);
      break;
  }
  return ACS_STATUS_ERR;
}