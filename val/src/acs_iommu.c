
#include "include/bsa_acs_val.h"
#include "include/bsa_acs_iommu.h"
#include "include/bsa_acs_common.h"

IOMMU_INFO_TABLE  *g_iommu_info_table;

/**
  @brief   This API executes all the IOMMU tests sequentially
           1. Caller       -  Application layer.
           2. Prerequisite -  val_iommu_create_info_table()
  @param   num_hart - the number of HART to run these tests on.
  @param   g_sw_view - Keeps the information about which view tests to be run
  @return  Consolidated status of all the tests run.
**/
uint32_t
val_iommu_execute_tests(uint32_t num_hart, uint32_t *g_sw_view)
{
  uint32_t status, i;

  for (i = 0; i < g_num_skip; i++) {
      if (g_skip_test_num[i] == ACS_IOMMU_TEST_NUM_BASE) {
          val_print(ACS_PRINT_INFO, "\n       USER Override - Skipping all IOMMU tests\n", 0);
          return ACS_STATUS_SKIP;
      }
  }

  /* Check if there are any tests to be executed in current module with user override options*/
  status = val_check_skip_module(ACS_IOMMU_TEST_NUM_BASE);
  if (status) {
      val_print(ACS_PRINT_INFO, "\n       USER Override - Skipping all IOMMU tests\n", 0);
      return ACS_STATUS_SKIP;
  }

  val_print_test_start("IOMMU");
  status      = ACS_STATUS_PASS;
  g_curr_module = 1 << IOMMU_MODULE;

  if (g_sw_view[G_SW_OS]) {
      val_print(ACS_PRINT_ERR, "\nOperating System View:\n", 0);
      status |= os_iom001_entry(num_hart);
  }
  val_print_test_end(status, "IOMMU");

  return status;
}

/**
  @brief   This API will call PAL layer to fill in the IOMMU information
           into the g_iommu_info_table pointer.
           1. Caller       -  Application layer.
           2. Prerequisite -  Memory allocated and passed as argument.
  @param   iommu_info_table  pre-allocated memory pointer for iommu_info
  @return  Error if Input param is NULL
**/
uint32_t
val_iommu_create_info_table(uint64_t *iommu_info_table)
{
  if (iommu_info_table == NULL) {
      val_print(ACS_PRINT_ERR, "Input for Create Info table cannot be NULL\n", 0);
      return ACS_STATUS_ERR;
  }
  val_print(ACS_PRINT_INFO, " Creating IOMMU INFO table\n", 0);

  g_iommu_info_table = (IOMMU_INFO_TABLE *)iommu_info_table;

  pal_iommu_create_info_table(g_iommu_info_table);
  val_print(ACS_PRINT_INFO, " RV porting: IOMMU info print to be added\n", 0);

  return ACS_STATUS_PASS;
}

/**
  @brief   This API frees the memory assigned for iommu info table
           1. Caller       -  Application Layer
           2. Prerequisite -  val_iommu_create_info_table
  @param   None
  @return  None
**/
void
val_iommu_free_info_table(void)
{
  pal_mem_free((void *)g_iommu_info_table);
}

/**
  @brief   This API returns the number of IOMMU from the g_iommu_info_table.
           1. Caller       -  Application layer, test Suite.
           2. Prerequisite -  val_hart_create_info_table.
  @param   none
  @return  the number of hart discovered
**/
uint32_t
val_iommu_get_num()
{
  if (g_iommu_info_table == NULL) {
      return 0;
  }
  return g_iommu_info_table->header.num_of_iommu;
}

/**
  @brief   This API is a single point of entry to retrieve
           information stored in the IOMMU Info table
           1. Caller       -  Test Suite
           2. Prerequisite -  val_hart_create_info_table
  @param   index  IOMMU index in the table
  @param   type   the type of information being requested
  @return  64-bit data
**/
uint64_t
val_iommu_get_info(int32_t index, IOMMU_INFO_e info_type)
{

  if (g_iommu_info_table == NULL)
      return 0;

  switch (info_type) {
    case IOMMU_INFO_TYPE:
        return g_iommu_info_table->iommu_info[index].type;
    case IOMMU_INFO_BASE_ADDRESS:
        return g_iommu_info_table->iommu_info[index].base_address;
    default:
      return 0;
  }
}