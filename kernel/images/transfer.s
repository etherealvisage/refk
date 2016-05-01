[BITS 64]
[ORG 0xffffffffffe00000]

task_percpu_region	equ 0xffffffffffe01000
task_percpu_size_log	equ 6
task_percpu_size	equ (1 << task_percpu_size_log)

; Expected as input:
;	rdi: points to task state structure to store state into
; 	rsi: points to task state structure to switch into
transfer_control:
	cmp	rdi, 0
	je	.skip_save

	; save GPRs
	mov	qword [rdi + 0*8], rax
	mov	qword [rdi + 1*8], rbx
	mov	qword [rdi + 2*8], rcx
	mov	qword [rdi + 3*8], rdx
	mov	qword [rdi + 4*8], rsi
	mov	qword [rdi + 5*8], rdi
	; remove return pointer from saved stack
	mov	rax, rsp
	add	rax, 8
	mov	qword [rdi + 6*8], rax
	mov	qword [rdi + 7*8], rbp
	mov	qword [rdi + 8*8], r8
	mov	qword [rdi + 9*8], r9
	mov	qword [rdi + 10*8], r10
	mov	qword [rdi + 11*8], r11
	mov	qword [rdi + 12*8], r12
	mov	qword [rdi + 13*8], r13
	mov	qword [rdi + 14*8], r14
	mov	qword [rdi + 15*8], r15

	; rflags
	pushfq
	pop	rax
	mov	qword [rdi + 16*8], rax
	; rip
	mov	rax, qword [rsp]
	mov	qword [rdi + 17*8], rax

	xor	rax, rax
	mov	ax, cs
	mov	qword [rdi + 18*8], rax
	mov	ax, ds
	mov	qword [rdi + 19*8], rax
	mov	ax, es
	mov	qword [rdi + 20*8], rax
	mov	ax, fs
	mov	qword [rdi + 21*8], rax
	mov	ax, gs
	mov	qword [rdi + 22*8], rax
	mov	ax, ss
	mov	qword [rdi + 23*8], rax
	; save FS_BASE
	mov	ecx, 0xc0000100 ; FS_BASE MSR
	rdmsr
	mov	dword [rdi + 24*8], eax
	mov	dword [rdi + 24*8 + 4], edx
	; save GS_BASE
	mov	ecx, 0xc0000101 ; GS_BASE MSR
	rdmsr
	mov	dword [rdi + 25*8], eax
	mov	dword [rdi + 25*8 + 4], edx

	mov	rax, cr3
	mov	qword [rdi + 26*8], rax

.skip_save:
	; finished storing everything

	; swap paging structures if required
	mov	rbx, qword [rsi + 26*8]
	cmp	rax, rbx
	je	.skip_swap

	mov	cr3, rbx
.skip_swap:
	; get task percpu storage region
	rdtscp
	shl	ecx, task_percpu_size_log
	lea	rax, [task_percpu_region + ecx]

	; save "restored-into" task state pointer
	mov	qword [rax], rsi
	; use rest of region for stack
	lea	rsp, [rax + task_percpu_size]

	;mov	qword [task_percpu_region], rsi
	;mov	rsp, task_percpu_region + 256

	; restore basic segment registers
	mov	rax, qword [rsi + 20*8] ; es
	mov	es, ax
	mov	rax, qword [rsi + 21*8] ; fs
	mov	fs, ax
	mov	rax, qword [rsi + 22*8] ; gs
	mov	gs, ax

	; restore FS_BASE
	mov	ecx, 0xc0000100 ; FS_BASE MSR
	mov	eax, dword [rsi + 24*8]
	mov	edx, dword [rsi + 24*8 + 4]
	wrmsr
	; restore GS_BASE
	mov	ecx, 0xc0000101 ; GS_BASE MSR
	mov	eax, dword [rsi + 25*8]
	mov	edx, dword [rsi + 25*8 + 4]
	wrmsr

	; restore GPRs except for rax, rsi, rsp
	mov	rbx, qword [rsi + 1*8]
	mov	rcx, qword [rsi + 2*8]
	mov	rdx, qword [rsi + 3*8]
	mov	rdi, qword [rsi + 5*8]
	mov	rbp, qword [rsi + 7*8]
	mov	r8, qword [rsi + 8*8]
	mov	r9, qword [rsi + 9*8]
	mov	r10, qword [rsi + 10*8]
	mov	r11, qword [rsi + 11*8]
	mov	r12, qword [rsi + 12*8]
	mov	r13, qword [rsi + 13*8]
	mov	r14, qword [rsi + 14*8]
	mov	r15, qword [rsi + 15*8]


	; stack push order: SS, RSP, RFLAGS, CS, RIP
	mov	rax, qword [rsi + 23*8] ; ss
	push	rax
	mov	rax, qword [rsi + 6*8] ; rsp
	push	rax
	mov	rax, qword [rsi + 16*8] ; rflags
	push	rax
	mov	rax, qword [rsi + 18*8] ; cs
	push	rax
	mov	rax, qword [rsi + 17*8] ; rip
	push	rax

	; save rax on the stack
	mov	rax, qword [rsi + 0*8]
	push	rax

	; we have to make sure ds is still valid here for all the accesses,
	; hence this weird ordering
	mov	rax, qword [rsi + 19*8] ; ds
	; restore rsi
	mov	rsi, qword [rsi + 4*8]
	mov	ds, ax

	pop	rax ; pop the stored value of rax

	iretq
