[BITS 32]

extern kernel_pbase

section .mb_header progbits
; Multiboot header values.

; Load on page-aligned boundaries . . .
MODULEALIGN	equ	1<<0
; Provide memory map . . .
MEMINFO		equ	1<<1
; Specify loading address manually . . .
LOAD_ADDR	equ	1<<16
; Flags for multiboot header.
FLAGS		equ	MODULEALIGN | MEMINFO | LOAD_ADDR
; Magic number for multiboot header.
MAGIC		equ	0x1BADB002
; Checksum to verify values of magic and flags.
CHECKSUM	equ	-(MAGIC + FLAGS)

; Actual multiboot header.
multiboot_header:
	dd	MAGIC
	dd	FLAGS
	dd	CHECKSUM
	; Header address.
	dd	multiboot_header
	; Load address; beginning of useful information.
	dd	bootstrap
	; Load ending address. (0, load the entire file.)
	dd	0
	; BSS ending address. (0, no BSS segment. We'll handle the BSS.)
	dd	0
	; Entry point.
	dd	bootstrap

section .mb_text progbits
global bootstrap
; Bootstrap function: called from Multiboot-compatible bootloader.
; Requires: eax to be set to 0x2badb002, ebx to point to the MB info structure.
bootstrap:
	; Disable interrupts.
	cli

	; Check to make sure that this was indeed loaded by a Multiboot loader.
	; Subtract the Multiboot magic signature from eax.
	sub	eax, 0x2badb002
	test	eax, eax
	; If the result is not zero, jump to the error symbol.
	jnz	error

	; Set up the stack.
	mov	esp, bootstrap_stack
	mov	ebp, esp

	; Extract any useful information from the multiboot header.
	call	header_extract

	; Set up paging.
	call	setup_paging

	; Set up long mode . . .
	call	setup_lmode

	; Now, jump to 64-bit code!
	jmp	0x08:realm64

align 8
GDT:				; Global Descriptor Table (64-bit).
	.null: equ $ - GDT	; The null descriptor.
	dw 0			; Limit (low).
	dw 0			; Base (low).
	db 0			; Base (middle)
	db 0			; Access.
	db 0			; Granularity.
	db 0			; Base (high).
	.code: equ $ - GDT	; The code descriptor.
	dw 0			; Limit (low).
	dw 0			; Base (low).
	db 0			; Base (middle)
	db 10011000b		; Access.
	db 00100000b		; Granularity.
	db 0			; Base (high).
	.data: equ $ - GDT	; The data descriptor.
	dw 0			; Limit (low).
	dw 0			; Base (low).
	db 0			; Base (middle)
	db 10010010b		; Access.
	db 00000000b		; Granularity.
	db 0			; Base (high).
	.pointer:		; The GDT-pointer.
	dw $ - GDT - 1		; Limit.
	dq GDT			; Base.

[BITS 64]

extern kmain

; Used to call kmain(). Does nothing else but provide a wrapper to call
; high-memory code from low-memory code.
realm64:
	; Set up segment registers.
	mov	ax, 0x10
	mov	ss, ax
	mov	ds, ax
	mov	es, ax
	mov	fs, ax
	mov	gs, ax

	; Call kmain().
	; First, set up the arguments.
	; Calling convention uses rdi as the first argument.
	mov	rdi, bootstrap_map
	mov	r9, kmain
	jmp	r9

[BITS 32]

; Extracts any useful/relevant information from the Multiboot header.
; Requires: ebx to point to the beginning of the Multiboot info structure.
; Returns: Nothing directly; fills in the values of various global symbols.
header_extract:
	; First step: usable memory ranges.
	mov	ecx, dword [ebx + 44]		; Grab the size . . .
	mov	esi, dword [ebx + 48]		; . . . and the base address.

	add	ecx, esi			; Find the end address.

	mov	edi, bootstrap_map		; Set up edi.
.mem_loop:
	; Check if this is the end of the buffer.
	mov	eax, ecx
	sub	eax, esi
	test	eax, eax
	jz	.mem_loop_end

	; Check the type. If != 1, skip this element.
	mov	eax, dword [esi + 20]
	sub	eax, 1
	test	eax, eax
	jnz	.mem_loop_skip

	; Copy the start address over.
	mov	eax, dword [esi + 4]
	stosd

	mov	eax, dword [esi + 8]
	stosd

	; Now the size.
	mov	eax, dword [esi + 12]
	stosd

	mov	eax, dword [esi + 16]
	stosd
.mem_loop_skip:
	add	esi, [esi]		; Increment esi by the size of this memory info element.
	add	esi, 4			; Plus the size of the size.
	jmp	.mem_loop		; Loop.

.mem_loop_end:
	; Finish off the list with an empty entry (16 zeroes).
	xor	eax, eax
	mov	ecx, 4
	rep	stosd

	ret

; Initializes PAE paging, maps the kernel into memory, etc.
; Returns: nothing.
setup_paging:
	; Housekeeping: zero out all paging structures.
	mov	ecx, paging_data_end
	sub	ecx, paging_data_start
	; Size is a multiple of 4096, so can zero out by dwords w/o problems.
	shr	ecx, 2
	xor	eax, eax
	mov	edi, paging_data_start
	rep	stosd

	; We'll begin with the ID mappings for the bootstrap code and the
	; first 2MB of RAM.

	; Begin from the top of the tree by adding the ID P3 into the P4.
	mov	dword [paging_p4 + 0*8], paging_id_p3 + 0x03
	; Add the ID P2 into the ID P3.
	mov	dword [paging_id_p3 + 0*8], paging_id_p2 + 0x03
	; Add the ID P1s into the ID P2.
	; Start with low-memory, at entry 0.
	mov	dword [paging_id_p2 + 0*8], paging_id_low_p1 + 0x03
	; Continue with bootstrap.
	mov	dword [paging_id_p2 + 8*8], paging_id_bootstrap_p1 + 0x03

	; Construct the P1 tables. Start with the low-memory map...
	xor	esi, esi
	mov	edi, paging_id_low_p1
.low_map:
	; store physical address | 0x03.
	mov	eax, esi
	or	eax, 0x3
	stosd
	; advance cursor over second dword
	mov	eax, 0
	stosd
	; increment cursor by 4KB
	add	esi, 0x1000
	; are we done?
	cmp 	esi, 0x200000
	jl	.low_map

	; Continue with the bootstrap map...
	mov	esi, kernel_pbase
	mov	edi, paging_id_bootstrap_p1
.bootstrap_map:
	; store physical address | 0x03.
	mov	eax, esi
	or	eax, 0x3
	stosd
	; advance cursor over second dword
	mov	eax, 0
	stosd
	; increment cursor by 4KB
	add	esi, 0x1000
	; are we done?
	cmp 	esi, 0x1200000
	jl	.bootstrap_map

	; Now, time to make the kernel mappings.
	; We have two things that we need to load: the code, and the data. Code
	; is mapped read-only and data mapped no-execute.
	; Grab the symbols so we know where to start/end mapping.
extern kernel_pbase
extern _code_phy_begin
extern _code_phy_end
extern _data_phy_begin
extern _data_phy_end
	; We'd like to use 2MB pages, but that makes the kernel image ugly and
	; padded. So instead we'll use 4KB pages but limit ourselves to 2MB of
	; mapped memory, for simplicity.
	; Thus, for code, we want the very simple flags of 0x01: present,
	; read-only, and in supervisor mode. For data, 0x03: present, writable,
	; and in supervisor mode. Note that at some point we should add the NX
	; bit to the kernel data.

	; We know that the phy begin and end is aligned to a 4KB page boundary,
	; simply because the linker script is set up that way. Let's start the
	; annoying part, which is linking together all the higher-level paging
	; structures.

	; Our starting address is 0xffffffff80000000. That means that we're in
	; the very last 2GB, so we want to put ourselves into the last P4 slot.
	; We use permissive flags for now.
	mov	eax, paging_high_p3
	or	eax, 3
	mov	dword [paging_p4 + 511*8], eax

	; Now, each entry in the P3 represents 1GB, so we want the second-last
	; entry for our P2 table.
	mov	eax, paging_high_p2
	or	eax, 3
	mov	dword [paging_high_p3 + 510*8], eax

	; Our basic offset is 0 into the GB bracket, so we want the first entry
	; in the P2 table for our P1.
	mov	eax, paging_high_p1
	or	eax, 3
	mov	dword [paging_high_p2 + 0*8], eax

	mov	esi, _code_phy_begin
	mov	edi, paging_high_p1

	mov	ecx, _code_phy_begin
	sub	ecx, kernel_pbase
	shr	ecx, 9

	add	edi, ecx

	; Now it's time to map enough 4KB pages to cover everything.

	cld
	; map until we've mapped enough.
.code_map:
	; store physical address | 0x01.
	mov	eax, esi
	or	eax, 0x1
	stosd
	; advance cursor over second dword
	mov	eax, 0
	stosd
	; increment cursor by 4KB
	add	esi, 0x1000
	; are we done?
	cmp 	esi, _code_phy_end
	jl	.code_map

	; Code's mapped in. Time for data. As before, using 4KB pages; however,
	; the flags are a little different because we want this to be writable.
	; As mentioned before, the flags are 0x03 (present, writable) and
	; there's the no-execute flag set.
	mov	esi, _data_phy_begin

	; map until we've mapped enough.
.data_map:
	; store physical address | 0x03.
	mov	eax, esi
	or	eax, 0x03
	stosd
	; store no-exec bit
	mov	eax, (1<<31) ; no-execute
	stosd
	; increment cursor by 4KB
	add	esi, 0x1000
	; are we done?
	cmp 	esi, _data_phy_end
	jl	.data_map

	; Set up physical memory mapping.
	; Map the four GB via four P3 entries.
	mov	dword [paging_phy_p3 + 0*8], 0x83 + 0x00000000
	mov	dword [paging_phy_p3 + 0*8 + 4], 0
	mov	dword [paging_phy_p3 + 1*8], 0x83 + 0x40000000
	mov	dword [paging_phy_p3 + 1*8 + 4], 0
	mov	dword [paging_phy_p3 + 2*8], 0x83 + 0x80000000
	mov	dword [paging_phy_p3 + 2*8 + 4], 0
	mov	dword [paging_phy_p3 + 3*8], 0x83 + 0xc0000000
	mov	dword [paging_phy_p3 + 3*8 + 4], 0

	; Point the correct entry in the P4 to the physical memory map.
	mov	eax, paging_phy_p3
	or	eax, 3
	mov	dword [paging_p4 + 384*8], eax

	; Finally, fill in cr3 with the address of the P4.
	mov	eax, paging_p4
	mov	cr3, eax

	ret

; Sets up long mode, brings CPU into compatibility mode.
setup_lmode:
	; Set PAE paging bit and PGE (global pages) bit.
	mov	eax, cr4
	or	eax, (1 << 5) + (1 << 7)
	mov	cr4, eax

	; Set the 9th bit of the EFER MSR to enable long mode.
	mov	ecx, 0xc0000080 ; EFER
	rdmsr
	or	eax, 1<<8
	; while we're at it, enable the NX bit
	or	eax, 1<<11
	wrmsr

	; Actually enable paging . . .
	mov	eax, cr0
	or	eax, 1<<31
	; Enable write-protection (CR0.WP) while we're at it.
	or 	eax, 1<<16
	mov	cr0, eax

	; Load the new (provisional, will be replaced later) GDT.
	lgdt	[GDT.pointer]

	; All that's left is to actually jump into 64-bit code, which will be done elsewhere.
	ret

error:
	hlt
	jmp	$

section .mb_bss nobits
	resb	2048			; Reserve a 2KB stack.
bootstrap_stack:			; Bootstrap stack top.
bootstrap_map:				; Memory for the bootstrap memory map structure.
	resb	2048
; Paging structures.
align	4096
paging_data_start:
paging_p4:
	resq	512
paging_id_p3:
	resq	512
paging_id_p2:
	resq	512
paging_id_low_p1:
	resq	512
paging_id_bootstrap_p1:
	resq	512
paging_high_p3:
	resq	512
paging_high_p2:
	resq	512
paging_high_p1:
	resq	512
paging_phy_p3:
	resq	512
paging_data_end:
