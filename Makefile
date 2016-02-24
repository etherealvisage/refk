QEMU_PATH=qemu-system-x86_64
QEMU_FLAGS=-m 64 -curses

.PHONY: all
all: kernel

.PHONY: kernel
kernel:
	$(MAKE) -C kernel

.PHONY: qemu-mb
qemu-mb: kernel
	TERM=xterm $(QEMU_PATH) $(QEMU_FLAGS) -kernel kernel/kernel.bin

.PHONY: clean
clean:
	-rm -f $(ISO_TARGET)
	$(MAKE) -C kernel clean

