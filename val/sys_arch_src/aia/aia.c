#include "include/bsa_acs_val.h"
#include "include/bsa_acs_iic.h"
#include "include/bsa_acs_common.h"

#include "aia.h"

#define IMSIC_EIPx_BITS			32
#define IMSIC_EIDELIVERY		0x70
#define IMSIC_EITHRESHOLD		0x72
#define IMSIC_EIP0			    0x80
#define IMSIC_EIE0			    0xc0

#define imsic_csr_write(__c, __v)	\
do { \
	csr_write(CSR_SISELECT, __c); \
	csr_write(CSR_SIREG, __v); \
} while (0)

#define imsic_csr_read(__c)	\
({ \
	unsigned long __v; \
	csr_write(CSR_SISELECT, __c); \
	__v = csr_read(CSR_SIREG); \
	__v; \
})

#define imsic_csr_set(__c, __v)		\
do { \
	csr_write(CSR_SISELECT, __c); \
	csr_set(CSR_SIREG, __v); \
} while (0)

#define imsic_csr_clear(__c, __v)	\
do { \
	csr_write(CSR_SISELECT, __c); \
	csr_clear(CSR_SIREG, __v); \
} while (0)

void
val_iic_imsic_eix_array_update (uint32_t base_id, uint32_t num_id, bool pend, bool val)
{
	uint32_t i, isel, ireg;
	uint32_t id = base_id, last_id = base_id + num_id;

	while (id < last_id) {
		isel = id / __riscv_xlen;
		isel *= __riscv_xlen / IMSIC_EIPx_BITS;
		isel += (pend) ? IMSIC_EIP0 : IMSIC_EIE0;

		ireg = 0;
		for (i = id & (__riscv_xlen - 1);
		     (id < last_id) && (i < __riscv_xlen); i++) {
			ireg |= BIT(i);
			id++;
		}

		if (val)
			imsic_csr_set(isel, ireg);
		else
			imsic_csr_clear(isel, ireg);
	}
}

void
val_iic_imsic_eix_update (uint32_t id, bool pend, bool val)
{
	uint32_t isel, ireg;

    isel = id / __riscv_xlen;
    isel *= __riscv_xlen / IMSIC_EIPx_BITS;
    isel += (pend) ? IMSIC_EIP0 : IMSIC_EIE0;

    ireg = 0;
    ireg |= BIT(id & (__riscv_xlen - 1));

    if (val) {
        // val_print(ACS_PRINT_INFO, "\n           imsic_csr_set(0x%x)", isel);
        imsic_csr_set(isel, ireg);
    } else {
        // val_print(ACS_PRINT_INFO, "\n           imsic_csr_clear(0x%x)", isel);
        imsic_csr_clear(isel, ireg);
    }
}

uint64_t
val_iic_imsic_eix_read (uint32_t id, bool pend)
{
    uint32_t isel;

    isel = id / __riscv_xlen;
    isel *= __riscv_xlen / IMSIC_EIPx_BITS;
    isel += (pend) ? IMSIC_EIP0 : IMSIC_EIE0;

    // val_print(ACS_PRINT_INFO, "\n           imsic_csr_read(0x%x)", isel);
    return imsic_csr_read(isel);
}

uint32_t
val_iic_imsic_eidelivery_update (uint32_t val)
{
	imsic_csr_write(IMSIC_EIDELIVERY, val);
    return imsic_csr_read(IMSIC_EIDELIVERY);
}

uint32_t
val_iic_imsic_eithreshold_update (uint32_t val)
{
	imsic_csr_write(IMSIC_EITHRESHOLD, val);
    return imsic_csr_read(IMSIC_EITHRESHOLD);
}