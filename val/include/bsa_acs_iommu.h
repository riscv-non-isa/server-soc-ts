#ifndef __BSA_ACS_IOMMU_H__
#define __BSA_ACS_IOMMU_H__

#include <stdbool.h>

#define EFI_ACPI_6_5_RIMT_DEVICE_TYPE_IOMMU               0
#define EFI_ACPI_6_5_RIMT_DEVICE_TYPE_PCIE_ROOT_COMPLEX   1
#define EFI_ACPI_6_5_RIMT_DEVICE_TYPE_Platform_DEVICE     2

uint32_t
os_iom001_entry(uint32_t num_hart);

#endif
