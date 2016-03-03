CC=gcc
NASM=nasm
XXD=xxd

CFLAGS=-W -Wall -Wextra -nostdlib -nodefaultlibs -mcmodel=large -m64 \
	-ffreestanding -mno-red-zone -masm=intel -std=c99 \
	-Werror -Wno-error=unused-variable -Wno-error=unused-function \
	-Wno-error=unused-parameter \
	-I`pwd`
LDFLAGS=-nostdlib -nodefaultlibs -m elf_x86_64

KERNEL_OUTPUT=kernel.bin

LDSCRIPT=kernel.ld

KERNEL_SOURCES=$(wildcard *.c) $(wildcard *.s)

#IMAGE_SOURCES:=$(wildcard images/*.s)
#IMAGES:=$(IMAGE_SOURCES:.s=.h)

OBJECTS:=$(KERNEL_SOURCES:.c=.o)
OBJECTS:=$(OBJECTS:.s=.o)

.PHONY: all
all: $(KERNEL_OUTPUT)

.c.o:
	$(CC) $(CFLAGS) -c $< -o $@
.s.o:
	$(NASM) -f elf64 $< -o $@

images/transfer.h: images/transfer.s
	$(NASM) -f bin images/transfer.s -o images/transfer
	$(XXD) -i images/transfer > images/transfer.h

#.PHONY: images
#images: $(IMAGES)

$(KERNEL_OUTPUT): $(OBJECTS) $(LOADER_SOURCES) $(LDSCRIPT)
	$(LD) $(LDFLAGS) -T $(LDSCRIPT) -o kernel.bin $(OBJECTS)

depend: $(KERNEL_SOURCES)
	gcc -MM $(CFLAGS) $(KERNEL_SOURCES) > depend
-include depend

.PHONY: clean
clean:
	-rm -f $(OBJECTS) $(KERNEL_OUTPUT) depend
