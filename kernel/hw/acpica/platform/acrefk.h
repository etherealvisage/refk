/******************************************************************************
 *
 * Name: aclinux.h - OS specific defines, etc. for refk
 *
 *****************************************************************************/

/*
 * Copyright (C) 2000 - 2016, Intel Corp.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions, and the following disclaimer,
 *    without modification.
 * 2. Redistributions in binary form must reproduce at minimum a disclaimer
 *    substantially similar to the "NO WARRANTY" disclaimer below
 *    ("Disclaimer") and any redistribution must be conditioned upon
 *    including a substantially similar Disclaimer requirement for further
 *    binary redistribution.
 * 3. Neither the names of the above-listed copyright holders nor the names
 *    of any contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * Alternatively, this software may be distributed under the terms of the
 * GNU General Public License ("GPL") version 2 as published by the Free
 * Software Foundation.
 *
 * NO WARRANTY
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * HOLDERS OR CONTRIBUTORS BE LIABLE FOR SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGES.
 */

#ifndef __ACREFK_H__
#define __ACREFK_H__

#include <stdint.h>

#include "klib/synch.h"

/* Common (in-kernel/user-space) ACPICA configuration */

//#define ACPI_USE_SYSTEM_CLIBRARY
#define ACPI_USE_DO_WHILE_0


//#define ACPI_USE_SYSTEM_INTTYPES

/* Kernel specific ACPICA configuration */

#ifdef CONFIG_ACPI_REDUCED_HARDWARE_ONLY
#define ACPI_REDUCED_HARDWARE 1
#endif

/* Host-dependent types and defines for in-kernel ACPICA */

#define ACPI_MACHINE_WIDTH          64
//#define ACPI_EXPORT_SYMBOL(symbol)  symbol
#define ACPI_EXPORT_SYMBOL(symbol)  
//#define strtoul                     simple_strtoul

#define ACPI_CACHE_T                ACPI_MEMORY_LIST
#define ACPI_USE_LOCAL_CACHE        1
#define ACPI_SPINLOCK               spinlock_t *
#define ACPI_SEMAPHORE              semaphore_t *
#define ACPI_CPU_FLAGS              unsigned long

#define ACPI_MISALIGNMENT_NOT_SUPPORTED

/*
 * Overrides for in-kernel ACPICA
 */
//#define ACPI_USE_ALTERNATE_PROTOTYPE_AcpiOsInitialize
//#define ACPI_USE_ALTERNATE_PROTOTYPE_AcpiOsTerminate
//#define ACPI_USE_ALTERNATE_PROTOTYPE_AcpiOsAllocate
//#define ACPI_USE_ALTERNATE_PROTOTYPE_AcpiOsAllocateZeroed
//#define ACPI_USE_ALTERNATE_PROTOTYPE_AcpiOsFree
//#define ACPI_USE_ALTERNATE_PROTOTYPE_AcpiOsAcquireObject
//#define ACPI_USE_ALTERNATE_PROTOTYPE_AcpiOsGetThreadId
//#define ACPI_USE_ALTERNATE_PROTOTYPE_AcpiOsCreateLock

/*
 * OSL interfaces used by debugger/disassembler
 */
//#define ACPI_USE_ALTERNATE_PROTOTYPE_AcpiOsReadable
//#define ACPI_USE_ALTERNATE_PROTOTYPE_AcpiOsWritable

/*
 * OSL interfaces used by utilities
 */
//#define ACPI_USE_ALTERNATE_PROTOTYPE_AcpiOsRedirectOutput
//#define ACPI_USE_ALTERNATE_PROTOTYPE_AcpiOsGetTableByName
//#define ACPI_USE_ALTERNATE_PROTOTYPE_AcpiOsGetTableByIndex
//#define ACPI_USE_ALTERNATE_PROTOTYPE_AcpiOsGetTableByAddress
//#define ACPI_USE_ALTERNATE_PROTOTYPE_AcpiOsOpenDirectory
//#define ACPI_USE_ALTERNATE_PROTOTYPE_AcpiOsGetNextFilename
//#define ACPI_USE_ALTERNATE_PROTOTYPE_AcpiOsCloseDirectory

/* Linux uses GCC */

#include "acgcc.h"

#endif /* __ACREFK_H__ */
