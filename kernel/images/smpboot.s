[BITS 16]

[ORG 0x4000]

_entry:
	jmp	rmode

align 32

id:		dd 0
paging_struct:	dd 0
entry_point:	dq 0

rmode:
	; set up GDT
	lgdt	[GDT.pointer]

	; enable protected mode
	mov	eax, cr0
	or	al, 1
	mov	cr0, eax

	jmp	0x18:pmode
[BITS 32]
pmode:
	; use 32-bit ds/ss
	mov	ax, 0x20
	mov	ds, ax
	mov	ss, ax

	; begin long mode init

	; enable PAE, global pages
	mov	eax, cr4
	or	eax, 1<<5
	mov	cr4, eax

	mov	eax, cr4
	or	eax, 1<<7
	mov	cr4, eax

	; set LM + NX EFER bits
	mov	ecx, 0xc0000080
	rdmsr
	or	eax, (1<<8) + (1<<11)
	wrmsr
	
	; set cr3
	mov	eax, [paging_struct]
	mov	cr3, eax

	; enable paging, WP
	mov	eax, cr0
	or	eax, (1<<31)
	mov	cr0, eax

	mov	eax, cr0
	or	eax, (1<<16)
	mov	cr0, eax

	jmp	0x08:lmode
[BITS 64]
lmode:
	; use 64-bit ds/cs
	mov	ax, 0x10
	mov	ds, ax
	mov	ss, ax

	mov	rsp, stack
	sub	rsp, 16
	xor	rdi, rdi
	mov	edi, dword [id]
	mov	r9, qword [entry_point]

	jmp	r9

; temporary GDT, will be replaced with kernel-wide one later
GDT:				; Global Descriptor Table (64-bit).
	.null: equ $ - GDT	; The null descriptor.
	dw 0			; Limit (low).
	dw 0			; Base (low).
	db 0			; Base (middle)
	db 0			; Access.
	db 0			; Granularity.
	db 0			; Base (high).
	.code64: equ $ - GDT	; The 64-bit code descriptor.
	dw 0			; Limit (low).
	dw 0			; Base (low).
	db 0			; Base (middle)
	db 10011000b		; Access.
	db 00100000b		; Granularity.
	db 0			; Base (high).
	.data64: equ $ - GDT	; The 64-bit data descriptor.
	dw 0			; Limit (low).
	dw 0			; Base (low).
	db 0			; Base (middle)
	db 10010010b		; Access.
	db 00000000b		; Granularity.
	db 0			; Base (high).
	.code32: equ $ - GDT    ; The 32-bit code descriptor.
	dw 0xffff		; Limit (low).
	dw 0			; Base (low).
	db 0			; Base (middle).
	db 10011000b		; Access.
	db 11001111b		; Granularity/limit (high).
	db 0			; Base (high).
	.data32: equ $ - GDT    ; The 32-bit data descriptor.
	dw 0xffff		; Limit (low).
	dw 0			; Base (low).
	db 0			; Base (middle).
	db 10010010b		; Access.
	db 11001111b		; Granularity/limit (high).
	db 0			; Base (high).
	.pointer:		; The GDT-pointer.
	dw $ - GDT - 1		; Limit.
	dd GDT			; Base.

align 0x1000
stack:
