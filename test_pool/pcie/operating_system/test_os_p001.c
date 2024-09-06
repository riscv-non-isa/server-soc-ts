/** @file
 * Copyright (c) 2016-2018, 2021-2023 Arm Limited or its affiliates. All rights reserved.
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

#include "val/include/bsa_acs_pcie.h"
#include "val/include/bsa_acs_hart.h"

#define TEST_NUM   (ACS_PCIE_TEST_NUM_BASE + 1)
#define TEST_RULE  "MF_ECM_010_010"
#define TEST_DESC  "ECAM Region accessibility check  "

static void *branch_to_test;

static
void
esr(uint64_t interrupt_type, void *context)
{
  uint32_t hart_index;

  hart_index = val_hart_get_index_mpid(val_hart_get_mpid());

  /* Update the ELR to return to test specified address */
  val_hart_update_elr(context, (uint64_t)branch_to_test);

  val_print(ACS_PRINT_INFO, "\n       Received exception of type: %d", interrupt_type);
  val_set_status(hart_index, RESULT_FAIL(TEST_NUM, 1));
}

/**
 * @brief 1. Parse ACPI MCFG tables to local all ECAM ranges.
 *        2. For each 4 KiB range in the ECAM range, verify that the following reads
 *           do not cause any errors or exceptions.
 *           a. 4-bytes at offset 0 - vendor and device ID
 *           b. 2-bytes at offset 0 - vendor ID
 *           c. 1 byte at offset 8 - revision ID
 */
static
void
payload(void)
{

  uint8_t  data8;
  uint16_t data16;
  uint32_t data32;
  uint32_t num_ecam;
  uint64_t ecam_base;
  uint32_t index;
  uint32_t bdf = 0;
  uint32_t bus, segment;
  uint32_t end_bus;
  uint32_t bus_index;
  uint32_t dev_index;
  uint32_t func_index;
  uint32_t ret;
  uint32_t status;

  index = val_hart_get_index_mpid(val_hart_get_mpid());

  /* Install sync and async handlers to handle exceptions.*/
  status = val_hart_install_esr(EXCEPT_AARCH64_SYNCHRONOUS_EXCEPTIONS, esr);
  status |= val_hart_install_esr(EXCEPT_AARCH64_SERROR, esr);
  if (status)
  {
      val_print(ACS_PRINT_ERR, "\n      Failed in installing the exception handler", 0);
      val_set_status(index, RESULT_FAIL(TEST_NUM, 1));
      return;
  }

  branch_to_test = &&exception_return;

  num_ecam = val_pcie_get_info(PCIE_INFO_NUM_ECAM, 0);
  if (num_ecam == 0) {
      val_print(ACS_PRINT_DEBUG, "\n       No ECAM in MCFG                   ", 0);
      val_set_status(index, RESULT_SKIP(TEST_NUM, 1));
      return;
  }

  while (num_ecam) {
      num_ecam--;
      ecam_base = val_pcie_get_info(PCIE_INFO_ECAM, num_ecam);
      if (ecam_base == 0) {
          val_print(ACS_PRINT_ERR, "\n       ECAM Base in MCFG is 0            ", 0);
          val_set_status(index, RESULT_SKIP(TEST_NUM, 1));
          return;
      }

      segment = val_pcie_get_info(PCIE_INFO_SEGMENT, num_ecam);
      bus = val_pcie_get_info(PCIE_INFO_START_BUS, num_ecam);
      end_bus = val_pcie_get_info(PCIE_INFO_END_BUS, num_ecam);


      /* Accessing the BDF PCIe config range */
      for (bus_index = bus; bus_index <= end_bus; bus_index++) {
        for (dev_index = 0; dev_index < PCIE_MAX_DEV; dev_index++) {
           for (func_index = 0; func_index < PCIE_MAX_FUNC; func_index++) {

               bdf = PCIE_CREATE_BDF(segment, bus_index, dev_index, func_index);
               ret = val_pcie_read_cfg_width(bdf, TYPE01_VIDR, &data32, PCI_WIDTH_UINT32);
               if (ret == PCIE_NO_MAPPING || (data32 == 0)) {
                  val_print(ACS_PRINT_ERR,
                        "\n         Incorrect vendor and device ID 0x%08x    ", data32);
                  val_set_status(index, RESULT_FAIL(TEST_NUM, (bus_index << 8)|dev_index));
                  return;
               }

               ret = val_pcie_read_cfg_width(bdf, TYPE01_VIDR, &data16, PCI_WIDTH_UINT16);
               if (ret == PCIE_NO_MAPPING || (data16 == 0)) {
                  val_print(ACS_PRINT_ERR,
                        "\n         Incorrect vendor ID %04x    ", data16);
                  val_set_status(index, RESULT_FAIL(TEST_NUM, (bus_index << 8)|dev_index));
                  return;
               }

               ret = val_pcie_read_cfg_width(bdf, TYPE01_RIDR, &data8, PCI_WIDTH_UINT8);
               if (ret == PCIE_NO_MAPPING) {
                  val_print(ACS_PRINT_ERR,
                        "\n         Incorrect revision 0x%02x    ", data8);
                  val_set_status(index, RESULT_FAIL(TEST_NUM, (bus_index << 8)|dev_index));
                  return;
               }
            }
        }
      }
  }

  val_set_status(index, RESULT_PASS(TEST_NUM, 1));

exception_return:
  return;
}

uint32_t
os_p001_entry(uint32_t num_hart)
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
