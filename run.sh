#!/bin/sh

if [[ $1 == "debug" ]]; then
    EXTRA_PARAMS="-d int,cpu_reset 2>ints"
fi

# environment variables
export TERM=xterm

MB_PARAMS="-m 64 -M q35"
KERNEL_PARAMS="-kernel kernel/kernel.bin"
NIC_PARAMS="-net nic,model=rtl8139"
NET_PARAMS="-net user"
HD_PARAMS="-drive file=testdisk,index=0,media=disk,format=raw"

sh -c "${QEMU_PATH}qemu-system-x86_64 -curses $MB_PARAMS $KERNEL_PARAMS $NIC_PARAMS $NET_PARAMS $HD_PARAMS $EXTRA_PARAMS"
