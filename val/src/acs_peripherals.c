/** @file
 * Copyright (c) 2016-2018, 2021-2022 Arm Limited or its affiliates. All rights reserved.
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
#include "include/bsa_acs_peripherals.h"
#include "include/bsa_acs_common.h"
#include "include/bsa_acs_pcie.h"

PERIPHERAL_INFO_TABLE  *g_peripheral_info_table;

/**
  @brief  Sequentially execute all the peripheral tests
          1. Caller       - Application
          2. Prerequisite - val_peripheral_create_info_table
  @param  num_pe - number of PEs to run this test on
  @param  g_sw_view - Keeps the information about which view tests to be run

  @result  consolidated status of all the tests
**/
uint32_t
val_peripheral_execute_tests(uint32_t num_pe, uint32_t *g_sw_view)
{

  uint32_t status, i;

  status = ACS_STATUS_PASS;

  for (i=0 ; i<MAX_TEST_SKIP_NUM ; i++){
      if (g_skip_test_num[i] == ACS_PER_TEST_NUM_BASE) {
          val_print(ACS_PRINT_TEST, "\n       USER Override - Skipping all Peripheral tests \n", 0);
          return ACS_STATUS_SKIP;
      }
  }

  if (g_single_module != SINGLE_MODULE_SENTINEL && g_single_module != ACS_PER_TEST_NUM_BASE &&
       (g_single_test == SINGLE_MODULE_SENTINEL ||
         (g_single_test - ACS_PER_TEST_NUM_BASE > 100 ||
          g_single_test - ACS_PER_TEST_NUM_BASE < 0))) {
    val_print(ACS_PRINT_TEST, "\n      USER Override - Skipping all Peripheral tests ", 0);
    val_print(ACS_PRINT_TEST, "\n      (Running only a single module)\n", 0);
    return ACS_STATUS_SKIP;
  }

  g_curr_module = 1 << PERIPHERAL_MODULE;

  if (g_sw_view[G_SW_OS]) {
      val_print(ACS_PRINT_ERR, "\nOperating System View:\n", 0);
#ifndef TARGET_LINUX
      status |= os_d001_entry(num_pe);
      status |= os_d002_entry(num_pe);
      status |= os_d003_entry(num_pe);
      status |= os_d005_entry(num_pe);
#else
      status |= os_d004_entry(num_pe);
#endif
  }

  if (status != ACS_STATUS_PASS)
    val_print(ACS_PRINT_TEST, "\n      *** One or more tests have Failed/Skipped.*** \n", 0);
  else
    val_print(ACS_PRINT_TEST, "\n       All Peripheral tests passed!! \n", 0);

  return status;
}

/**
  @brief  Return the Index of the entry in the peripheral info table
          which matches the input type and the input instance number
          Instance number is '0' based
          1. Caller       - VAL
          2. Prerequisite - val_peripheral_create_info_table
  @param  type     - Type of peripheral whose index needs to be returned
  @param  instance - Instance number is '0' based.

  @result  Index of peripheral matching type and instance
**/

uint32_t
val_peripheral_get_entry_index(uint32_t type, uint32_t instance)
{
  uint32_t  i = 0;

  while (g_peripheral_info_table->info[i].type != 0xFF) {
      if (type == PERIPHERAL_TYPE_NONE || g_peripheral_info_table->info[i].type == type) {
          if (instance == 0)
             return i;
          else
             instance--;

      }
      i++;
  }
  return 0xFFFF;
}

/**
  @brief  Single entry point to return all Peripheral related information.
          1. Caller       - Test Suite
          2. Prerequisite - val_peripheral_create_info_table
  @param  info_type - Type of peripheral whose index needs to be returned
  @param  instance  - id (0' based) for which the info has to be returned

  @result  64-bit data of peripheral matching type and instance
**/
uint64_t
val_peripheral_get_info(PERIPHERAL_INFO_e info_type, uint32_t instance)
{
  uint32_t i;

  if (g_peripheral_info_table == NULL)
      return 0;

  switch(info_type) {
      case NUM_USB:
          return g_peripheral_info_table->header.num_usb;
      case NUM_SATA:
          return g_peripheral_info_table->header.num_sata;
      case NUM_UART:
          return g_peripheral_info_table->header.num_uart;
      case NUM_ALL:
          return g_peripheral_info_table->header.num_all;
      case USB_BASE0:
          i = val_peripheral_get_entry_index(PERIPHERAL_TYPE_USB, instance);
          if (i != 0xFFFF)
              return g_peripheral_info_table->info[i].base0;
          break;
      case USB_FLAGS:
          i = val_peripheral_get_entry_index(PERIPHERAL_TYPE_USB, instance);
          if (i != 0xFFFF)
              return g_peripheral_info_table->info[i].flags;
          break;
      case USB_GSIV:
          i = val_peripheral_get_entry_index(PERIPHERAL_TYPE_USB, instance);
          if (i != 0xFFFF)
              return g_peripheral_info_table->info[i].irq;
          break;
      case USB_BDF:
          i = val_peripheral_get_entry_index(PERIPHERAL_TYPE_USB, instance);
          if (i != 0xFFFF)
              return g_peripheral_info_table->info[i].bdf;
          break;
      case USB_INTERFACE_TYPE:
          i = val_peripheral_get_entry_index(PERIPHERAL_TYPE_USB, instance);
          if (i != 0xFFFF)
              return g_peripheral_info_table->info[i].interface_type;
          break;
      case USB_PLATFORM_TYPE:
          i = val_peripheral_get_entry_index(PERIPHERAL_TYPE_USB, instance);
          if (i != 0xFFFF)
              return g_peripheral_info_table->info[i].platform_type;
          break;
      case SATA_BASE0:
          i = val_peripheral_get_entry_index(PERIPHERAL_TYPE_SATA, instance);
          if (i != 0xFFFF)
              return g_peripheral_info_table->info[i].base0;
          break;
      case SATA_BASE1:
          i = val_peripheral_get_entry_index(PERIPHERAL_TYPE_SATA, instance);
          if (i != 0xFFFF)
              return g_peripheral_info_table->info[i].base1;
          break;
      case SATA_FLAGS:
          i = val_peripheral_get_entry_index(PERIPHERAL_TYPE_SATA, instance);
          if (i != 0xFFFF)
              return g_peripheral_info_table->info[i].flags;
          break;
      case SATA_BDF:
          i = val_peripheral_get_entry_index(PERIPHERAL_TYPE_SATA, instance);
          if (i != 0xFFFF)
              return g_peripheral_info_table->info[i].bdf;
          break;
      case SATA_GSIV:
          i = val_peripheral_get_entry_index(PERIPHERAL_TYPE_SATA, instance);
          if (i != 0xFFFF)
              return g_peripheral_info_table->info[i].irq;
          break;
      case SATA_INTERFACE_TYPE:
          i = val_peripheral_get_entry_index(PERIPHERAL_TYPE_SATA, instance);
          if (i != 0xFFFF)
              return g_peripheral_info_table->info[i].interface_type;
          break;
      case SATA_PLATFORM_TYPE:
          i = val_peripheral_get_entry_index(PERIPHERAL_TYPE_SATA, instance);
          if (i != 0xFFFF)
              return g_peripheral_info_table->info[i].platform_type;
          break;
      case UART_BASE0:
          i = val_peripheral_get_entry_index(PERIPHERAL_TYPE_UART, instance);
          if (i != 0xFFFF)
              return g_peripheral_info_table->info[i].base0;
          break;
      case UART_WIDTH:
          i = val_peripheral_get_entry_index(PERIPHERAL_TYPE_UART, instance);
          if (i != 0xFFFF)
              return g_peripheral_info_table->info[i].width;
          break;
      case UART_GSIV:
          i = val_peripheral_get_entry_index(PERIPHERAL_TYPE_UART, instance);
          if (i != 0xFFFF)
              return g_peripheral_info_table->info[i].irq;
          break;
      case UART_FLAGS:
          i = val_peripheral_get_entry_index(PERIPHERAL_TYPE_UART, instance);
          if (i != 0xFFFF)
              return g_peripheral_info_table->info[i].flags;
          break;
      case UART_BAUDRATE:
          i = val_peripheral_get_entry_index(PERIPHERAL_TYPE_UART, instance);
          if (i != 0xFFFF)
              return g_peripheral_info_table->info[i].baud_rate;
          break;
      case UART_INTERFACE_TYPE:
          i = val_peripheral_get_entry_index(PERIPHERAL_TYPE_UART, instance);
          if (i != 0xFFFF)
              return g_peripheral_info_table->info[i].interface_type;
          break;
      case ANY_FLAGS:
          i = val_peripheral_get_entry_index (PERIPHERAL_TYPE_NONE, instance);
          if (i != 0xFFFF)
              return g_peripheral_info_table->info[i].flags;
          break;
      case ANY_GSIV:
          i = val_peripheral_get_entry_index (PERIPHERAL_TYPE_NONE, instance);
          if (i != 0xFFFF)
            return g_peripheral_info_table->info[i].irq;
          break;
      case ANY_BDF:
          i = val_peripheral_get_entry_index (PERIPHERAL_TYPE_NONE, instance);
          if (i != 0xFFFF)
              return g_peripheral_info_table->info[i].bdf;
          break;
      case MAX_PASIDS:
          i = val_peripheral_get_entry_index (PERIPHERAL_TYPE_NONE, instance);
          if (i != 0xFFFF)
              return g_peripheral_info_table->info[i].max_pasids;
          break;
      default:
          break;
  }
  return 0;
}

void
val_peripheral_dump_info(void)
{

  uint32_t bus, dev, func, seg = 0;
  uint32_t dev_bdf;
  uint32_t reg_value, base_cc;
  uint32_t dply = 0, ntwk = 0, strg = 0;

  if (val_pcie_get_info(PCIE_INFO_NUM_ECAM, 0) == 0)
  {
      val_print(ACS_PRINT_DEBUG, "\n No ECAM is present", 0);
      return;
  }

  for (bus = 0; bus < PCIE_MAX_BUS; bus++)
  {
      for (dev = 0; dev < PCIE_MAX_DEV; dev++)
      {
          for (func = 0; func < PCIE_MAX_FUNC; func++)
          {
              dev_bdf = PCIE_CREATE_BDF(seg, bus, dev, func);
              val_pcie_read_cfg(dev_bdf, TYPE01_VIDR, &reg_value);
              if (reg_value == PCIE_UNKNOWN_RESPONSE)
                  continue;
              val_pcie_read_cfg(dev_bdf, TYPE01_RIDR, &reg_value);
              val_print(ACS_PRINT_DEBUG, "\n BDF is %x", dev_bdf);
              val_print(ACS_PRINT_DEBUG, "\n Class code is %x", reg_value);
              base_cc = reg_value >> TYPE01_BCC_SHIFT;
              if (base_cc == CNTRL_CC)
                  ntwk++;
              if (base_cc == DP_CNTRL_CC)
                  dply++;
              if (base_cc == MAS_CC)
                  strg++;
              else
                  continue;
          }
      }
  }

  val_print(ACS_PRINT_DEBUG, " Peripheral: Num of Network ctrl      :    %d \n", ntwk);
  val_print(ACS_PRINT_DEBUG, " Peripheral: Num of Storage ctrl      :    %d \n", strg);
  val_print(ACS_PRINT_DEBUG, " Peripheral: Num of Display ctrl      :    %d \n", dply);

}

/*
 * val_create_peripheralinfo_table:
 *    Caller         Application layer.
 *    Prerequisite   Memory allocated and passed as argument.
 *    Description    This function will call PAL layer to fill all relevant peripheral
 *                   information into the g_peripheral_info_table pointer.
 */
/**
  @brief  This API calls PAL layer to fill all relevant peripheral
          information into the g_peripheral_info_table pointer
          1. Caller       - Application layer
          2. Prerequisite - Memory allocated and passed as argument
  @param  info_table - pointer to a memory where peripheral data is filled

  @result  None
**/

void
val_peripheral_create_info_table(uint64_t *peripheral_info_table)
{

  g_peripheral_info_table = (PERIPHERAL_INFO_TABLE *)peripheral_info_table;
  val_print(ACS_PRINT_INFO, " Creating PERIPHERAL INFO table\n", 0);

  pal_peripheral_create_info_table(g_peripheral_info_table);

  val_print(ACS_PRINT_TEST, " Peripheral: Num of USB controllers   :    %d \n",
    val_peripheral_get_info(NUM_USB, 0));
  val_print(ACS_PRINT_TEST, " Peripheral: Num of SATA controllers  :    %d \n",
    val_peripheral_get_info(NUM_SATA, 0));
  val_print(ACS_PRINT_TEST, " Peripheral: Num of UART controllers  :    %d \n",
    val_peripheral_get_info(NUM_UART, 0));
  val_peripheral_dump_info();

}


/**
  @brief  Free the memory allocated for Peripheral Info table

  @param  None

  @return None
 **/
void
val_peripheral_free_info_table()
{
  pal_mem_free((void *)g_peripheral_info_table);
}

/**
  @brief   Check if PCI device is PCI Express capable
           1. Caller       -  Test Suite
  @param   bdf      - PCIe BUS/Device/Function
  @return  1 -> PCIe capable  0 -> no PCIe capable
**/
uint32_t val_peripheral_is_pcie(uint32_t bdf)
{
  uint32_t seg  = PCIE_EXTRACT_BDF_SEG (bdf);
  uint32_t bus  = PCIE_EXTRACT_BDF_BUS (bdf);
  uint32_t dev  = PCIE_EXTRACT_BDF_DEV (bdf);
  uint32_t func = PCIE_EXTRACT_BDF_FUNC (bdf);

  return pal_peripheral_is_pcie(seg, bus, dev, func);
}

