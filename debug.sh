#!/bin/sh
TERM=xterm qemu-system-x86_64 -m 64 -curses -kernel kernel/kernel.bin -d int,cpu_reset 2>ints -serial file:serial -net nic,model=rtl8139 -net user -hda testdisk -M q35
