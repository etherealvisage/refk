#!/usr/bin/env python

import sys

def colourize(env):
    colors = {}
    colors["cyan"]   = '\033[96m'
    colors["purple"] = '\033[95m'
    colors["blue"]   = '\033[94m'
    colors["green"]  = '\033[92m'
    colors["yellow"] = '\033[93m'
    colors["red"]    = '\033[91m'
    colors["end"]    = '\033[0m'

    #If the output is not a terminal, remove the colors
    if not sys.stdout.isatty():
       for key, value in colors.iteritems():
          colors[key] = ''

    compile_source_message = '%sCompiling %s==> %s$SOURCE%s' % \
        (colors['blue'], colors['purple'], colors['yellow'], colors['end'])

    compile_shared_source_message = '%sCompiling shared %s==> %s$SOURCE%s' % \
        (colors['blue'], colors['purple'], colors['yellow'], colors['end'])

    link_program_message = '%sLinking Program %s==> %s$TARGET%s' % \
        (colors['red'], colors['purple'], colors['yellow'], colors['end'])

    link_library_message = '%sLinking Static Library %s==> %s$TARGET%s' % \
        (colors['red'], colors['purple'], colors['yellow'], colors['end'])

    ranlib_library_message = '%sRanlib Library %s==> %s$TARGET%s' % \
        (colors['red'], colors['purple'], colors['yellow'], colors['end'])

    link_shared_library_message = '%sLinking Shared Library %s==> %s$TARGET%s' % \
        (colors['red'], colors['purple'], colors['yellow'], colors['end'])
    env.Replace(CXXCOMSTR = compile_source_message)
    env.Replace(CCCOMSTR = compile_source_message)
    env.Replace(SHCCCOMSTR = compile_shared_source_message)
    env.Replace(SHCXXCOMSTR = compile_shared_source_message)
    env.Replace(ARCOMSTR = link_library_message)
    env.Replace(RANLIBCOMSTR = ranlib_library_message)
    env.Replace(SHLINKCOMSTR = link_shared_library_message)
    env.Replace(LINKCOMSTR = link_program_message)

def builders(env):
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

    env.Append(BUILDERS = {"Image": imager})
    env.Append(BUILDERS = {"Assemble": assembler})
    env.Append(BUILDERS = {"FlatBinary": flat_assembler})
    env.Append(BUILDERS = {"KernelProc": kernel_proc})
    env.Append(BUILDERS = {"Kernel": kernel})

def gcc(env):
    env.Append(CFLAGS = "-std=c99")
    env.Append(CFLAGS = "-W -Wall -Wextra -nostdlib -nodefaultlibs -ffreestanding")
    env.Append(CFLAGS = "-mcmodel=large -m64 -mno-red-zone")
    env.Append(CFLAGS = "-Werror -Wno-error=unused-variable -Wno-error=unused-function")
    env.Append(CFLAGS = "-Wno-error=unused-parameter")
    env.Append(CFLAGS = "-I kernel/")
    env.Append(CFLAGS = "-fdiagnostics-color=always")

def clang(env):
    env.Replace(CC = "clang")
    env.Append(CFLAGS = "-std=c99")
    env.Append(CFLAGS = "-W -Wall -Wextra -nostdlib -ffreestanding")
    env.Append(CFLAGS = "-mcmodel=large -m64 -mno-red-zone")
    env.Append(CFLAGS = "-Werror -Wno-error=unused-variable -Wno-error=unused-function")
    env.Append(CFLAGS = "-Wno-error=unused-parameter")
    env.Append(CFLAGS = "-mno-sse -mno-mmx")
    env.Append(CFLAGS = "-I kernel/")

env = Environment(tools=["default", "nasm"])

builders(env)
if True:
    gcc(env)
else:
    clang(env)

# for non-debug versions
#env.Append(CFLAGS = "-DNDEBUG")

Export("env")

SConscript(dirs=["kernel", "rlib"])
