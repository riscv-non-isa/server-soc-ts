#/** @file
# Copyright (c) 2016-2018, Arm Limited or its affiliates. All rights reserved.
# SPDX-License-Identifier : Apache-2.0
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#  http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
#**/


/* Tool Support Status:
1. GCC support:  https://gcc.gnu.org/pipermail/gcc-patches/2022-May/595504.html
2. Qemu support: https://mail.gnu.org/archive/html/qemu-riscv/2023-02/msg00572.html
*/

.text
.align 3

GCC_ASM_EXPORT(DataCacheCleanInvalidateVA)
GCC_ASM_EXPORT(DataCacheInvalidateVA)
GCC_ASM_EXPORT(DataCacheCleanVA)

ASM_PFX(DataCacheCleanInvalidateVA):
#  dc  civac, x0
   cbo.flush 0(a0)
#  dsb sy
   fence 
#  isb
   fence.i 
/*
   1. CMO operations are not ordered by other instructions that order stores, e.g. FENCE.I and SFENCE.VMA.
   2. But fence.i will ensure the coherence between ICache and Dcache: https://github.com/riscv/riscv-CMOs/issues/61
*/
  ret

ASM_PFX(DataCacheCleanVA):
#  dc  cvac, x0
   cbo.clean 0(a0)
#  dsb ish
   fence 
#  isb
   fence.i
  ret

ASM_PFX(DataCacheInvalidateVA):
#  dc  ivac, x0
   cbo.inval 0(a0)
#  dsb ish
   fence
#  isb
   fence.i
  ret