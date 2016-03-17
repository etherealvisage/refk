#!/bin/sh

rm testdisk
dd if=/dev/zero of=testdisk bs=1M count=64
mkfs.vfat -F 32 testdisk
