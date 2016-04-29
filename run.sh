#!/bin/sh

if [[ $1 == "debug" ]]; then
    EXTRA_PARAMS="-serial file:serial -d int,cpu_reset"
fi

EXTRA_PARAMS="$EXTRA_PARAMS -monitor tcp:localhost:4444,server"

# environment variables
export TERM=xterm

MB_PARAMS="-m 64 -M q35 -smp 2 -cpu Haswell"
KERNEL_PARAMS="-kernel kernel/kernel.bin"
NIC_PARAMS="-net nic,model=rtl8139"
NET_PARAMS="-net user"
HD_PARAMS="-drive file=testdisk,index=0,media=disk,format=raw"

QEMU_COMMAND="${QEMU_PATH}qemu-system-x86_64 -curses $MB_PARAMS $KERNEL_PARAMS $NIC_PARAMS $NET_PARAMS $HD_PARAMS $EXTRA_PARAMS"
echo $QEMU_COMMAND
sh -c "$QEMU_COMMAND" 2>ints
