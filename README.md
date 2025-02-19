# RISC-V Server SoC Compliance Test Suite

## RISC-V Server SoC Compliance Test Suite

This suite was build on RISC-V Server SoC Test
Specification, **Version v0.51**.


## RISC-V Server SoC Spec

The RISC-V server SoC specification defines a standardized set of capabilities that portable system software such as operating systems and hypervisors, can rely on being present in a RISC-V server SoC. For the spec information, please refer to [https://github.com/riscv-non-isa/server-soc](https://github.com/riscv-non-isa/server-soc).


## RISC-V Server SoC Test Spec

Along with the Server SoC Spec, there is a test spec which defines a set of tests to verify if the requirements specified in RISC-V Server SoC specification are implemented. This test suite will be designed based on the test spec. For more information about the test spec, please also refer to [https://github.com/riscv-non-isa/server-soc](https://github.com/riscv-non-isa/server-soc).

### Other Repo dependence

|  Repos       |   URL                                           |   tag/commit                           | patch                       |
| -------      |  -----------------------------------            |  ---------------------                 | -------------------------   |
| edk2         | https://github.com/tianocore/edk2               |  edk2-stable202405                     | Add RiscVOpensbiLib, ACPI table parsers and BaseRiscV64CpuExceptionHandlerLib bug fix |
|edk2-platforms| https://github.com/tianocore/edk2-platforms.git |ba73190ddccc0d0e8c9ff4d3cac1f10bde8b0f71| Add RiscVQemuServerPlatform |
| qemu         | https://github.com/vlsunil/qemu.git             |  acpi_rimt_poc_v1                      |                             |
| opensbi      | https://github.com/riscv-software-src/opensbi.git | a731c7e36988c3308e1978ecde491f2f6182d490 |                         |


## Compile Server SoC TestSuite
### 1. Build Env
    Before you start the build, ensure that the following requirements are met.

- Ubuntu 22.04 (Currently Tested)
- Export your working folder as $WORKSPACE:
    ```
    export WORKSPACE=`pwd`
    ```
- git clone the [EDK2 tree](https://github.com/tianocore/edk2) to $WORKSPACE\edk2. Recommended edk2 tag is edk2-stable202405
- git clone the [EDK2-Platforms tree](https://github.com/tianocore/edk2-platforms.git) to $WORKSPACE\edk2-platforms. Recommended edk2-platforms commit is ba73190ddccc0d0e8c9ff4d3cac1f10bde8b0f71
- git clone the [EDK2 port of libc](https://github.com/tianocore/edk2-libc) $WORKSPACE\edk2-libc
- Install the build prerequisite packages to build EDK2.<br />
  ```
  apt install gcc-riscv64-linux-gnu acpica-tools \
    git curl mtools gdisk gcc openssl automake autotools-dev libtool \
    bison flex bc uuid-dev python3 \
    libglib2.0-dev libssl-dev autopoint libslirp-dev \
    make g++ gcc-riscv64-unknown-elf gettext \
    gcc-aarch64-linux-gnu
  ```

### 2. Clone server-soc-ts repo and Patch the EDK2
1.  cd $WORKSPACE\edk2
2.  git submodule update --init --recursive
3.  git clone git@github.com:riscv-non-isa/server-soc-ts.git ShellPkg/Application/server-soc-ts
4.  git apply ShellPkg/Application/server-soc-ts/patches/0001-Apply-patch-ShellPkg-Application-server-soc-ts-patch.patch

### 3. Patch the EDK2-Platforms
1.  cd $WORKSPACE\edk2-platforms
2.  git submodule update --init --recursive
3.  Checkout opensbi commmit a731c7e36988c3308e1978ecde491f2f6182d490
    ```
    cd Silicon/RISC-V/ProcessorPkg/Library/RiscVOpensbiLib/opensbi
    git checkout a731c7e36988c3308e1978ecde491f2f6182d490 -b rv-ts
    ```
5.  git apply ../edk2/ShellPkg/Application/server-soc-ts/patches/0001-RiscVQemuServerPlatform-Initial-commit-for-RISC-V-Qe.patch

### 4. Build the TestSuite under UEFI
1.  Use below script to build the TestSuite.
    ```
    export GCC5_RISCV64_PREFIX=riscv64-linux-gnu-
    export GCC5_AARCH64_PREFIX=aarch64-linux-gnu-
    export GCC49_AARCH64_PREFIX=aarch64-linux-gnu-
    export PACKAGES_PATH=$WORKSPACE/edk2:$WORKSPACE/edk2-platforms:$WORKSPACE/edk2-libc

    source edk2/edksetup.sh

    #Build basetool
    make -C BaseTools/Source/C

    #Build the TestSuite application
    source edk2/ShellPkg/Application/server-soc-ts/tools/scripts/acsbuild.sh
    ```
    The EFI executable file is generated at <edk2_path>/Build/Shell/DEBUG\_GCC5/RISCV64/Bsa.efi
2.  Package Bsa.efi into a disk image.
    ```
    dd if=/dev/zero of=disk.img bs=1M count=128

    # Update your mtoolsrc to map the disk.img to drive z before using below mformat and mcopy command
    mformat -i disk.img -t 4096 -h 64 -s 32 -F Z:
    mcopy -i disk.img -s ./Build/Shell/DEBUG_GCC5/RISCV64/Bsa.efi  ::/
    ```

### 5. Build RiscVQemuServerPlatform BIOS
1. Use below script to build the test BIOS
   ```
    export GCC5_RISCV64_PREFIX=riscv64-linux-gnu-
    export GCC5_AARCH64_PREFIX=aarch64-linux-gnu-
    export GCC49_AARCH64_PREFIX=aarch64-linux-gnu-
    export PACKAGES_PATH=$WORKSPACE/edk2:$WORKSPACE/edk2-platforms:$WORKSPACE/edk2-libc

    source edk2/edksetup.sh
    build -a RISCV64 -t GCC5 -p Platform/Qemu/RiscVQemuServerPlatform/RiscVQemuServerPlatform.dsc

    # truncate RV flash files
    truncate -s 32M Build/RiscVQemuServerPlatform/DEBUG_GCC5/FV/RISCV_SP_CODE.fd
    truncate -s 32M Build/RiscVQemuServerPlatform/DEBUG_GCC5/FV/RISCV_SP_VARS.fd
   ```

### 6. Build RISC-V QEMU
1. git clone the https://github.com/vlsunil/qemu.git to $QEMU_WORKSPACE/qemu, tag is  acpi_rimt_poc_v1
2. Use below script to build the test QEMU
   ```
   export QEMU_CODE=$QEMU_WORKSPACE/qemu
   export QEMU_INSTALL=$QEMU_WORKSPACE/qemu-install

   mkdir $QEMU_CODE/build
   cd $QEMU_CODE/build
   ../configure --enable-slirp --prefix=$QEMU_INSTALL
   make
   make install
   ```
### 7. Build OpenSBI binary
1.  Build OpenSBI binary as below.
   ```
   git clone https://github.com/riscv/opensbi.git opensbi
   make -C opensbi \
	    -j $(getconf _NPROCESSORS_ONLN) \
	    CROSS_COMPILE=riscv64-linux-gnu- \
	    PLATFORM=generic
   ```

### 8. Run the TestSuite under UEFI
1.  Mount disk.img as a drive in RISC-V QEMU virt platform and start the QEMU
    ```
    $QEMU_INSTALL/bin/qemu-system-riscv64 \
            -machine virt,aia=aplic-imsic,pflash0=pflash0,pflash1=pflash1 \
            -blockdev node-name=pflash0,driver=file,read-only=on,filename=$EDK2_WORKSPACE/Build/RiscVQemuServerPlatform/DEBUG_GCC5/FV/RISCV_SP_CODE.fd \
            -blockdev node-name=pflash1,driver=file,filename=$EDK2_WORKSPACE/Build/RiscVQemuServerPlatform/DEBUG_GCC5/FV/RISCV_SP_VARS.fd \
            -bios opensbi/build/platform/generic/firmware/fw_dynamic.bin \
            -drive file=$EDK2_WORKSPACE/disk.img,format=raw,if=virtio \
		    -serial stdio
    ```
2.  Boot to UEFI Shell and run Bsa.efi to start the test suite.
    ```
    fs0:
    Bsa.efi -v 1
    ```

## Current Test Result
Refer test_result/riscv_qemu_virt.md for EDK2 RISC-V QEMU virt platform test result.

    Status:
    * TBI - To be implemented，means a test is feasible in UEFI ACPI PAL but not implemented.
    * NA - Not applicable，means no test is needed\
    * Blocked - Test (or test result confirm) is blocked due to QEMU/FW/OS issue or missing features.
    * Pending - Need further investigation
--------------------------------------------------------------------------------------------

## License
RISC-V Server SoC Testsuit is distributed under Apache v2.0 License.

## Feedback, contributions, and support

* For feedback, use the GitHub Issue Tracker that is associated with this repository.
* Arm licensees may contact Arm directly through their partner managers.
* Arm welcomes code contributions through GitHub pull requests. See the GitHub documentation on how to raise pull requests.

--------------

*Copyright (c) 2021-2023, Arm Limited and Contributors. All rights reserved.*
