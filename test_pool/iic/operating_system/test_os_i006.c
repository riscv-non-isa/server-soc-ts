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
 #include "val/include/bsa_acs_hart.h"
 #include "val/include/bsa_acs_memory.h"

 #define TEST_NUM   (ACS_GIC_TEST_NUM_BASE + 6)
 #define TEST_RULE  "ME_IIC_080_010"
 #define TEST_DESC  "APLIC Functionality Test"


 #define GENMSI_OFFSET   			0x3000
 #define targeti_OFFSET  			0x3004
 #define DOMAINCFG_DM				0x4
 #define EIID						0x3

 #define HSTATUS_VGEIN_SHIFT		12
 #define HSTATUS_VGEIN				0x0003f000UL

 #define TARGET_GUEST_INDEX_SHIFT 	12
 #define TARGET_GUEST_INDEX 		0x0003f000UL

	/**
  * @brief  1. Parse ACPI MADT to determine if an APLIC for supervisor interrupt
              domain is reported.
            2. If no APLIC is reported then skip the remaining steps.
            3. Locate the APLIC structure.
            4. Verify that number of interrupt delivery control. structures is reported
              as 0 indicating it is used as a wired-to-MSI bridge.
            5. Verify the domaincfg supports MSI delivery mode and is configured to
              be in MSI delivery mode.
            6. Write an external interrupt ID to genmsi register and verify that the
              extempore MSI is delivered to the IMSIC of the targeted hart.
            7. Verify that the guest index field of the target[i] registers support all
              values between 0 and GEILEN supported by the IMSIC.
  */
 static
 void
 payload()
 {
   	uint32_t index = val_hart_get_index_mpid(val_hart_get_mpid());
   	uint64_t aplic_base, val;
	uint16_t idc_struct_cnt, ext_intr_src;
	uint32_t vgein;
	uint64_t val_w;
	uint64_t val_r;

  	/* Checkpoint 	1: Parse ACPI MADT to determine if an APLIC for supervisor interrupt
      					domain is reported.
					2. If no APLIC is reported then skip the remaining steps.
					3. Locate the APLIC structure.
	*/
	aplic_base = val_aplic_base();
  	if (aplic_base == 0){
		val_print(ACS_PRINT_ERR, "\n	APLIC base not found", 0);
		val_set_status(index, RESULT_FAIL(TEST_NUM, 1));
		return;
  	}
  	val_print(ACS_PRINT_INFO, "\n       APLIC base: 0x%lx", aplic_base);

  	/* Checkpoint 	4: Verify that number of interrupt delivery control. structures is reported
						as 0 indicating it is used as a wired-to-MSI bridge.
	*/
	idc_struct_cnt = val_aplic_idc_num();
	if (idc_struct_cnt != 0){
		val_print(ACS_PRINT_ERR, "\n	Reported IDC Structure Count: ", idc_struct_cnt);
		val_set_status(index, RESULT_FAIL(TEST_NUM, 4));
		return;
	}
	val_print(ACS_PRINT_INFO, "\n       Reported IDC Structure Count: %d", idc_struct_cnt);


  	/* Checkpoint 	5: Verify the domaincfg supports MSI delivery mode and is configured to
						be in MSI delivery mode.
	*/
	val_memory_map_add_mmio(aplic_base, 0x4000);
	val = val_mmio_read(aplic_base);
	if ((val & (DOMAINCFG_DM)) != DOMAINCFG_DM){
		val_print(ACS_PRINT_ERR, "\n       domaincfg is not configured for MSI delivery mode!", 0);
		val_set_status(index, RESULT_FAIL(TEST_NUM, 5));
		return;
	}
	val_print(ACS_PRINT_INFO, "\n       Reported domaincfg value - 0x%lx", val);

  	/* Checkpoint 	6: Write an external interrupt ID to genmsi register and verify that the
						extempore MSI is delivered to the IMSIC of the targeted hart.
	*/
	val_iic_imsic_eix_update(EIID, true, 0); //CLEAR the EIID.
	val_print(ACS_PRINT_INFO, "\n       Check pending bit in EIPk is CLEAR", 0);
	val = val_iic_imsic_eix_read(EIID, true);
	val_print(ACS_PRINT_INFO, "\n         EIPk: 0x%x", (val & EIID));
	//Write EIID=3 at the genmsi register.
	val_mmio_write(aplic_base + GENMSI_OFFSET, EIID);
	//verify the write by reading the register.
	val = val_mmio_read(aplic_base + GENMSI_OFFSET);
	if (EIID != val){
		val_print(ACS_PRINT_ERR, "\n       Not able to write genmsi register at address 0x%lx", aplic_base + GENMSI_OFFSET);
		val_print(ACS_PRINT_ERR, " with EIID %d", EIID);
		val_set_status(index, RESULT_FAIL(TEST_NUM, 6));
		return;
	}
	val_print(ACS_PRINT_ERR, "\n       Written genmsi register at address 0x%lx", aplic_base + GENMSI_OFFSET);
	val_print(ACS_PRINT_ERR, " with EIID %d", EIID);

	val_print(ACS_PRINT_INFO, "\n       Read and check pending bit in EIPk is set", 0);
	if (EIID != val){
		val = val_iic_imsic_eix_read(EIID, true);
		val_print(ACS_PRINT_INFO, "\n       Pending bit is not set in EIPK for EIID: %d", val);
		val_set_status(index, RESULT_FAIL(TEST_NUM, 6));
		return;
	}
	val_print(ACS_PRINT_INFO, "\n         EIPk: 0x%x", val);
	val_iic_imsic_eix_update(EIID, true, 0); //CLEAR the EIID.

  	/* Checkpoint 7: Verify that the guest index field of the target[i] registers support all
						values between 0 and GEILEN supported by the IMSIC.
	*/

	//Step 1: let's find the supported GEILEN using hstatus.
	for (vgein = 1; vgein <= 0x3F; vgein++) {
		val_w = (val_hart_get_hstatus() & (~HSTATUS_VGEIN)) | (vgein << HSTATUS_VGEIN_SHIFT);
		val_hart_set_hstatus(val_w);
		val_r = val_hart_get_hstatus();
		if ((val_w & HSTATUS_VGEIN) != (val_r & HSTATUS_VGEIN)) {
				break;
		}
	}
	val_print(ACS_PRINT_INFO, "\n      Valid GEILEN is: %d", vgein - 1);

	//Step 2: Check Total number of external interrupt sources available.
	ext_intr_src = val_aplic_extern_intr_src();
	val_print(ACS_PRINT_INFO, "\n       Total External Interrupt Sources are: %d", ext_intr_src);

	//Step 3: Find all these values are supported by the guest index field of target[i] registers.
	for (int i = 0; i < ext_intr_src; i++){
		for (val =1; val < vgein; val++){
			val_mmio_write(aplic_base + targeti_OFFSET + i*4, 0);											//Clear Target[i] register.
			val_mmio_write(aplic_base + targeti_OFFSET + i*4, (val << TARGET_GUEST_INDEX_SHIFT));			//Write Target[i] register.
			val_r = val_mmio_read(aplic_base + targeti_OFFSET + i*4);										//Read back the Target[i] register.
			if ((val_r & TARGET_GUEST_INDEX) != (val << TARGET_GUEST_INDEX_SHIFT)) {
				val_print(ACS_PRINT_INFO, "\n      GUEST INDEX of Target[%d]", i);
				val_print(ACS_PRINT_INFO, " does not support all the values between 0 and %d of GEILEN", vgein - 1);
				val_print(ACS_PRINT_INFO, "\n      Target[%d]", i);
				val_print(ACS_PRINT_INFO, " does not support writing %d at GUEST INDEX", val);
				val_set_status(index, RESULT_FAIL(TEST_NUM, 7));
				return;
			}
		}
	}

	val_print(ACS_PRINT_INFO, "\n       Target[0 ... %d]", ext_intr_src - 1);
	val_print(ACS_PRINT_INFO, " supports all the values from 0 to %d", vgein - 1);

	val_set_status(index, RESULT_PASS(TEST_NUM, 1));

 }

 uint32_t
 os_i006_entry(uint32_t num_hart)
 {

   uint32_t status = ACS_STATUS_FAIL;

   num_hart = 1;  //This IIC test is run on single processor

   status = val_initialize_test(TEST_NUM, TEST_DESC, num_hart);

   if (status != ACS_STATUS_SKIP)
       val_run_test_payload(TEST_NUM, num_hart, payload, 0);

   /* get the result from all HART and check for failure */
   status = val_check_for_error(TEST_NUM, num_hart, TEST_RULE);

   val_report_status(0, BSA_ACS_END(TEST_NUM), NULL);

   return status;
 }
