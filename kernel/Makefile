CC=gcc
NASM=nasm
XXD=xxd

CFLAGS=-W -Wall -Wextra -nostdlib -nodefaultlibs -mcmodel=large -m64 \
	-ffreestanding -mno-red-zone -masm=intel -std=c99 \
	-Werror -Wno-error=unused-variable -Wno-error=unused-function \
	-Wno-error=unused-parameter -I`pwd`
LDFLAGS=-nostdlib -nodefaultlibs -m elf_x86_64

KERNEL_OUTPUT=kernel.bin
KERNEL_LDSCRIPT=kernel.ld

KERNEL_SOURCES:=$(wildcard *.c) $(wildcard *.s)
KERNEL_OBJECTS:=$(KERNEL_SOURCES:.c=.o)
KERNEL_OBJECTS:=$(KERNEL_OBJECTS:.s=.o)

KLIB_SOURCES:=$(wildcard klib/*.c)
KLIB_OBJECTS:=$(KLIB_SOURCES:.c=.o)
KLIB_OUTPUT:=libklib.a

BOOT_SOURCES:=$(wildcard boot/*.c)
BOOT_OBJECTS:=$(BOOT_SOURCES:.c=.o)
BOOT_OUTPUT=boot.elf

PROC_LDSCRIPT=kproc.ld

.PHONY: all
all: $(KERNEL_OUTPUT)

.PHONY: clean
clean:
	-rm -f depend
	-rm -f images/transfer images/transfer.h
	-rm -f $(KLIB_OBJECTS) $(KLIB_OUTPUT)
	-rm -f images/boot.h $(BOOT_OBJECTS) $(BOOT_OUTPUT)
	-rm -f $(KERNEL_OBJECTS) $(KERNEL_OUTPUT)

depend: $(KERNEL_SOURCES) $(KLIB_SOURCES) $(BOOT_SOURCES) images/transfer.h images/boot.h
	gcc -MM $(CFLAGS) $(KERNEL_SOURCES) $(KLIB_SOURCES) $(BOOT_SOURCES) > depend
-include depend

$(KLIB_OUTPUT): $(KLIB_OBJECTS)
	-rm -f $(KLIB_OUTPUT)
	ar q $(KLIB_OUTPUT) $(KLIB_OBJECTS)

$(BOOT_OUTPUT): $(BOOT_OBJECTS) $(KLIB_OUTPUT) $(PROC_LDSCRIPT)
	$(LD) $(LDFLAGS) -T $(PROC_LDSCRIPT) -o $(BOOT_OUTPUT) $(BOOT_OBJECTS) -L. -lklib

images/transfer.h: images/transfer.s
	$(NASM) -f bin images/transfer.s -o images/transfer
	$(XXD) -i images/transfer > images/transfer.h

images/intr.h: images/intr.s
	$(NASM) -f bin images/intr.s -o images/intr
	$(XXD) -i images/intr > images/intr.h

images/boot.h: $(BOOT_OUTPUT)
	$(XXD) -i $(BOOT_OUTPUT) > images/boot.h

.c.o:
	$(CC) $(CFLAGS) -c $< -o $@
.s.o:
	$(NASM) -f elf64 $< -o $@

$(KERNEL_OUTPUT): images/transfer.h images/boot.h $(KERNEL_OBJECTS) $(LOADER_SOURCES) $(KERNEL_LDSCRIPT) 
	$(LD) $(LDFLAGS) -T $(KERNEL_LDSCRIPT) -o kernel.bin $(KERNEL_OBJECTS) -L. -lklib
