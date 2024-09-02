#include "val/include/bsa_acs_val.h"
#include "val/include/val_interface.h"
#include "val/include/bsa_acs_memory.h"

#include "val/include/bsa_acs_iommu.h"

#define TEST_NUM   (ACS_IOMMU_TEST_NUM_BASE + 1)
#define TEST_RULE  "ME_IOM_010_010"
#define TEST_DESC  "Check IOMMU capabilities register               "

#define RISCV_IOMMU_REG_CAP             0x0
#define RISCV_IOMMU_CAP_VERSION_MSK     0xFF
#define RISCV_IOMMU_CAP_VERSION_1_0     0x10

/**
 * @brief For each application processor hart:
 * 1. Locate all IOMMUs reported by APCI and verify they are of RIMT type.
 * 2. For each IOMMU, read the capabilities register and verify that it
 *    supports version 1.0 of the RISC-V IOMMU specification.
 * 3. Output the capabilities register in the test output log.
 */
static
void
payload()
{
  uint32_t index;
  uint32_t hart_index = val_hart_get_index_mpid(val_hart_get_mpid());
  uint32_t iommu_num = val_iommu_get_num();
  uint64_t base_addr, reg_cap;

  for (index = 0; index < iommu_num; index++) {
    if (val_iommu_get_info (index, IOMMU_INFO_TYPE) == EFI_ACPI_6_5_RIMT_DEVICE_TYPE_IOMMU) {
      base_addr = val_iommu_get_info (index, IOMMU_INFO_BASE_ADDRESS);

      /* Map the IOMMU memory-mapped register region */
      val_print(ACS_PRINT_INFO, "\n       IOMMU base: 0x%lx", base_addr);
      val_memory_map_add_mmio(base_addr, 0x1000);

      reg_cap = val_mmio_read64(base_addr + RISCV_IOMMU_REG_CAP);
      val_print(ACS_PRINT_INFO, "\n       IOMMU reg_cap - 0x%lx", reg_cap);
      if ((reg_cap & RISCV_IOMMU_CAP_VERSION_MSK) != RISCV_IOMMU_CAP_VERSION_1_0) {
        val_print(ACS_PRINT_ERR, "\n       Incorrect IOMMU version - 0x%x", (reg_cap & RISCV_IOMMU_CAP_VERSION_MSK));
        val_set_status(hart_index, RESULT_FAIL(TEST_NUM, 1));
        return;
      }
    }
  }

  val_set_status(hart_index, RESULT_PASS(TEST_NUM, 1));
}

uint32_t
os_iom001_entry(uint32_t num_hart)
{

  uint32_t status = ACS_STATUS_FAIL;

  num_hart = 1;  //This IOMMU test is run on single processor
  // TODO: test all processor

  status = val_initialize_test(TEST_NUM, TEST_DESC, num_hart);

  if (status != ACS_STATUS_SKIP)
      val_run_test_payload(TEST_NUM, num_hart, payload, 0);

  /* get the result from all HART and check for failure */
  status = val_check_for_error(TEST_NUM, num_hart, TEST_RULE);

  val_report_status(0, BSA_ACS_END(TEST_NUM), NULL);

  return status;
}
