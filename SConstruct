#!/usr/bin/env python

imager = Builder(action = "xxd -i < $SOURCE > $TARGET")
assembler = Builder(action = "nasm -f elf64 $SOURCE -o $TARGET")
flat_assembler = Builder(action = "nasm -f bin $SOURCE -o $TARGET")
def kernel_proc_generator(source, target, env, for_signature):
    source_list = ""
    for s in source:
        source_list += " " + str(s)
    base_cmd = "ld -nostdlib -nodefaultlibs -m elf_x86_64 "
    return base_cmd + source_list + " -T kernel/kproc.ld -o " + str(target[0])
kernel_proc = Builder(generator = kernel_proc_generator)
def kernel_generator(source, target, env, for_signature):
    source_list = ""
    for s in source:
        source_list += " " + str(s)
    base_cmd = "ld -nostdlib -nodefaultlibs -m elf_x86_64 "
    return base_cmd + source_list + " -T kernel/kernel.ld -o " + str(target[0])
kernel = Builder(generator = kernel_generator)

env = Environment(tools=["default", "nasm"])
env.Append(BUILDERS = {"Image": imager})
env.Append(BUILDERS = {"Assemble": assembler})
env.Append(BUILDERS = {"FlatBinary": flat_assembler})
env.Append(BUILDERS = {"KernelProc": kernel_proc})
env.Append(BUILDERS = {"Kernel": kernel})

env.Append(CFLAGS = "-std=c99")
env.Append(CFLAGS = "-W -Wall -Wextra -nostdlib -nodefaultlibs -ffreestanding")
env.Append(CFLAGS = "-mcmodel=large -m64 -mno-red-zone -masm=intel")
env.Append(CFLAGS = "-Werror -Wno-error=unused-variable -Wno-error=unused-function")
env.Append(CFLAGS = "-Wno-error=unused-parameter")
env.Append(CFLAGS = "-I kernel/")

Export("env")

SConscript(dirs=["kernel"])
