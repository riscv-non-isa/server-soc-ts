if [ $(uname -m) != "aarch64" ] && [ -v $GCC5_RISCV64_PREFIX ]
then
    echo "GCC49_AARCH64_PREFIX is not set"
    echo "set using export GCC49_AARCH64_PREFIX=<lib_path>/bin/aarch64-linux-gnu-"
    return 0
fi

if [ "$1" == "ENABLE_OOB" ]; then
    build -a RISCV64 -t GCC5 -p ShellPkg/ShellPkg.dsc -m ShellPkg/Application/server-soc-ts/baremetal_app/BsaAcs.inf -D ENABLE_OOB
    return 0;
fi

    build -a RISCV64 -t GCC5 -p ShellPkg/ShellPkg.dsc -m ShellPkg/Application/server-soc-ts/uefi_app/BsaAcs.inf
