/** @file
 * Copyright (c) 2016-2018, 2021, 2023, Arm Limited or its affiliates. All rights reserved.
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

#ifndef __BSA_ACS_IIC_H__
#define __BSA_ACS_IIC_H__

#include <stdbool.h>

#define BIT(nr)			(1ULL << (nr))

#define IMSIC_MMIO_PAGE_LE             0x00
#define IMSIC_MMIO_PAGE_BE             0x04

uint32_t
os_i001_entry(uint32_t num_hart);
uint32_t
os_i002_entry(uint32_t num_hart);
uint32_t
os_i003_entry(uint32_t num_hart);
uint32_t
os_i004_entry(uint32_t num_hart);
uint32_t
os_i005_entry(uint32_t num_hart);

void val_iic_imsic_eix_array_update (uint32_t base_id, uint32_t num_id, bool pend, bool val);
void val_iic_imsic_eix_update (uint32_t id, bool pend, bool val);
uint64_t val_iic_imsic_eix_read (uint32_t id, bool pend);
uint32_t val_iic_imsic_eidelivery_update (uint32_t val);
uint32_t val_iic_imsic_eithreshold_update (uint32_t val);

#endif
