[BITS 32]

section .mb_header
; Multiboot header values.

; Load on page-aligned boundaries . . .
MODULEALIGN	equ	1<<0
; Provide memory map . . .
MEMINFO		equ	1<<1
; Request video mode set
VIDEOMODE	equ 1<<2
; Specify loading address manually . . .
LOAD_ADDR	equ	1<<16
; Flags for multiboot header.
FLAGS		equ	MODULEALIGN | MEMINFO | LOAD_ADDR; | VIDEOMODE
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
	; BSS ending address. (0, no BSS segment.)
	dd	0
	; Entry point.
	dd	bootstrap
	; Video type. (0, linear graphics)
	dd 0
	; Video width.
	dd 1024
	; Video height
	dd 768
	; Video depth
	dd 24

section .text
global bootstrap
; Bootstrap function: called from Multiboot-compatible bootloader.
; Requires: eax to be set to 0x2badb002, ebx to point to the MB info structure.
bootstrap:
	cli				; Disable interrupts.
	
	; Check to make sure that this was indeed loaded by a Multiboot loader.
	sub	eax, 0x2badb002		; Subtract the Multiboot magic signature from eax.
	test	eax, eax		; Test eax.
	jnz	error			; If the result is not zero, jump to the error symbol.
	
	; Set up the stack.
	mov	esp, bootstrap_stack
	mov	ebp, esp
	
	; Set up the debugging line counter.
	mov	byte [bootstrap_last_line], 0x0

	; Extract any useful information from the multiboot header.
	call	header_extract

	; Set up paging.
	call	setup_paging

	; Set up long mode . . .
	call	setup_lmode

	; Now, jump to 64-bit code!
	jmp	0x08:realm64
	
	hlt
	jmp	$

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

; Used to call kmain(). Does nothing else but provide a wrapper to call high-memory code from low-memory code.
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
	ret

[BITS 32]

; Extracts any useful/relevant information from the Multiboot header.
; Requres: ebx to point to the beginning of the Multiboot info structure.
; Returns: Nothing directly; fills in the values of various global symbols.
header_extract:
	; Save ebx onto the stack for later use,
	push	ebx
	
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
	
	pushad
	call	debug_value
	popad
	
	mov	eax, dword [esi + 8]
	stosd
	
	; Now the size.
	mov	eax, dword [esi + 12]
	stosd
	
	pushad
	call	debug_value
	popad
	
	mov	eax, dword [esi + 16]
	stosd
.mem_loop_skip:
	add	esi, [esi]		; Increment esi by the size of this memory info element.
	add	esi, 4			; Plus the size of the size.
	jmp	.mem_loop		; Loop.
	
.mem_loop_end:
	pushad
	mov	eax, edi
	call	debug_value
	popad

	; Finish off the list with an empty entry (16 zeroes).
	xor	eax, eax
	mov	ecx, 4
	rep	stosd
	
	pop	ebx
	
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

	
	; Set up the ID mapping for the first 2MB using a 2MB page:
	;	0x00 is the base address, e.g. the first 2MB . . .
	;	0x03 states that the page is writable and present.
	;	0x80 states that this is a 2MB page.
	mov	dword [paging_id_pd + 0], (0x00 | 0x03 | 0x80)
	; Set up the loader 2MB page entry: (assumes that this assembly file
	;	does not exceed 2MB after compilation, which is reasonable.)
	;
	;	0x1000000 is the load address of the kernel;
	;	0x03 states that the page is writable and present.
	;	0x80 states that this is a 2MB page.
	mov	dword [paging_id_pd + 64], (0x1000000 | 0x03 | 0x80)
	; Add the PD to the PDPT.
	mov	eax, paging_id_pd
	or	eax, 3
	mov	dword [paging_id_pdpt], eax
	; PDPT into PML4 . . .
	mov	eax, paging_id_pdpt
	or	eax, 3
	mov	dword [paging_pml4], eax
	
	; Now, time to make the kernel mappings.
	; We have two things that we need to load: the code, and the data; code
	; is mapped read-only and data mapped no-execute.
	; Grab the symbols so we know where to start/end mapping.
extern kernel_pbase
extern _code_phy_begin
extern _code_phy_end
extern _data_phy_begin
extern _data_phy_end
	; We're going to use 2MB pages for this. So the flags are 0x81 (present
	; and 2MB page), in addition to bit 63 being set (no-execute).

	; Since 2MB pages have to have a 2MB-aligned target, things get
	; slightly messy here.
	
	; Instead of using the physical address of the code section starting
	; point, we need to use the kernel pbase to also map in the bootstrap
	; code (this).
	mov	esi, kernel_pbase
	mov	edi, paging_high_pd

	cld
	; map until we've mapped enough.
.code_map:
	; store physical address | 0x81.
	mov	eax, esi
	or	eax, 0x81
	stosd
	; advance cursor over second dword
	mov	eax, 0
	stosd
	; increment cursor by 2MB
	add	esi, 0x200000
	; are we done?
	cmp 	esi, _code_phy_end
	jl	.code_map

	; Code's mapped in. Time for data. As before, using 2MB pages; however,
	; the flags are a little different because we want this to be writable.
	; So the flags are 0x83 (present, writable, and 2MB page) and there's
	; no no-execute flag set.
	mov	esi, _data_phy_begin
	; Assume (dangerous!) that data starts on the next 2MB page after data.
	; So, no need to reset edi.

	; map until we've mapped enough.
.data_map:
	; store physical address | 0x83.
	mov	eax, esi
	or	eax, 0x83
	stosd
	; store no-exec bit
	mov	eax, 0 ;(1<<31) ; no-execute
	stosd
	; increment cursor by 2MB
	add	esi, 0x200000
	; are we done?
	cmp 	esi, _data_phy_end
	jl	.data_map
	
	; Okay, page directory set up. Now it's time to add the page directory
	; into the PDPT, and the PDPT into the PML4.

	; PD to the PDPT.
	mov	eax, paging_high_pd
	or	eax, 3
	mov	dword [paging_high_pdpt + 510*8], eax
	; PDPT to PML4 . . .
	mov	eax, paging_high_pdpt
	or	eax, 3
	mov	dword [paging_pml4 + 511*8], eax
	
	; Set up physical memory mapping.
	; Set up the PD.
	mov	eax, 0x83
	; Map the four GB via four PDs.
	mov	ecx, 2048
	mov	edi, paging_phy_pd
.phy_pdpt_loop:
	mov	dword [edi], eax
	mov	dword [edi + 4], 0
	add	edi, 8
	add	eax, 0x200000
	loop	.phy_pdpt_loop
	
	; Put the PDs into the PDPT.
	mov	eax, paging_phy_pd
	or	eax, 3
	mov	dword [paging_phy_pdpt], eax
	add	eax, 0x1000
	mov	dword [paging_phy_pdpt + 8], eax
	add	eax, 0x1000
	mov	dword [paging_phy_pdpt + 16], eax
	add	eax, 0x1000
	mov	dword [paging_phy_pdpt + 24], eax
	
	; Point the correct entry in the PML4 to the physical memory map.
	mov	eax, paging_phy_pdpt
	or	eax, 3
	mov	dword [paging_pml4 + 3072], eax
	
	; Finally, fill in cr3 with the address of the PML4.
	mov	eax, paging_pml4
	mov	cr3, eax
	
	ret


; Sets up long mode, brings CPU into compatibility mode.
setup_lmode:
	; Set PAE paging bit.
	mov	eax, cr4
	or	eax, 1 << 5
	mov	cr4, eax

	; Set PGE (global pages) bit.
	mov eax, cr4
	or eax, 1<<7
	mov cr4, eax
	
	; Set the 9th bit of the EFER MSR to enable long mode.
	mov	ecx, 0xc0000080
	rdmsr
	or	eax, 1<<8
	wrmsr
	
	; Actually enable paging . . .
	mov	eax, cr0
	or	eax, 1<<31
	; Enable write-protection (CR0.WP) while we're at it.
	or 	eax, 1<<16
	mov	cr0, eax

	; Load the new (provisional, will be replaced later) GDT.
	lgdt	[GDT.pointer]
	
	; Set up segment registers.
	;mov	ax, 0x10
	;mov	ds, ax
	
	; All that's left is to actually jump into 64-bit code, which will be done elsewhere.
	
	ret

; Prints a message to the next unused line on screen.
; Requires: eax to be set to the line number, esi to be set to the input string.
; Returns: nothing.
debug_message:
	pushad
	mov	edi, 0xb8000
	
	movzx	ebx, byte [bootstrap_last_line]
	
	inc	byte [bootstrap_last_line]
	
	mov	ebx, 160		; 160: number of bytes per 80-character line.
	mul	ebx
	add	edi, eax
	xor	eax, eax
.write_loop:
	mov	al, byte [esi]
	mov	byte [edi], al
	inc	esi
	inc	edi
	mov	byte [edi], 0x07
	inc	edi
	mov	al, byte [esi]
	test	al, al
	jnz	.write_loop

	popad
	ret


; Prints a value to the next unused line on the screen.
; Requires: eax to be set to the value to print.
; Returns: nothing.
debug_value:
	mov	edi, 0xb8000
	
	movzx	ebx, byte [bootstrap_last_line]
	inc	byte [bootstrap_last_line]
	
	mov	esi, eax		; Move the value into esi.
	
	mov	eax, 160		; 160: number of bytes per 80-character line.
	mul	ebx
	add	edi, eax
	
	mov	ecx, 28			; Value to shift by.
.digit_loop:
	mov	ebx, esi		; Copy the value to print.
	shr	ebx, cl			; Shift right.
	and	ebx, 0xf		; Bitwise and to mask out the other bits.
	; Look up the required character.
	mov	eax, hex_table
	add	eax, ebx
	mov	al, byte [eax]
	; Character now in al; use it.
	mov	byte [edi], al
	mov	byte [edi+1], 7
	add	edi, 2
	
	; Check the value of ecx.
	mov	eax, ecx
	test	eax, eax
	jz	.digit_loop_finished
	sub	ecx, 4
	jmp	.digit_loop
	
.digit_loop_finished:
	
	ret

error:
	mov	esi, error_string
	mov	eax, 0
	call	debug_message
	hlt
	jmp	$

section .rodata
loading_string:		db	"Loading . . ."
			db	0
found_string:		db	"Found memory map header."
			db	0
not_found_string:	db	"Could not find map header?"
			db	0
error_string:		db	"Error."
			db	0
hex_table:		db	"0123456789ABCDEF"   

section .bss
	resb	2048			; Reserve a 2KB stack.
bootstrap_stack:			; Bootstrap stack top.
bootstrap_map:				; Memory for the bootstrap memory map structure.
	resb	2048
; Paging structures.
align	4096
paging_data_start:
paging_pml4:
	resq	512
paging_id_pdpt:
	resq	512
paging_id_pd:
	resq	512
paging_high_pdpt:
	resq	512
paging_high_pd:
	resq	512
paging_phy_pdpt:
	resq	512
paging_phy_pd: ; maps the first four GB
	resq	2048
paging_data_end:

; Misc. variables.
bootstrap_last_line:
	resd	1
