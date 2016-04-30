[BITS 64]

[ORG 0xffffa00000000000]

%idefine sysretq o64 sysret

transfer_control	equ 0xffffffffffe00000
task_state_region	equ 0xffffffffffe10000
num_tasks		equ 4096 ; must be power of two
task_percpu_region	equ 0xffffffffffe01000
task_percpu_size_log	equ 6
task_percpu_size	equ (1 << task_percpu_size_log)

; Calling convention:
; Clobbered registers: rax, rcx, r10, r11, r12

syscall:
	; get safe stack

	; save rdx and rcx
	mov	r10, rdx
	mov	r12, rcx

	rdtscp
	shl	ecx, task_percpu_size_log
	add	rcx, task_percpu_region
	xchg	rsp, rcx

	; at this point, we have:
	; rax = killed
	; rcx = old rsp
	; r10 = orig rdx
	; r11 = orig r11 (old eflags)
	; r12 = orig rcx (old RIP)
	; rsp = percpu process state

	; percpu established, save old task if relevant
	pop	rax
	cmp	rax, 0
	xchg	rax, rdi
	je	.skip_save
	; need starting index

	; save GPRs
	mov	qword [rdi + 0*8], 0x1234567 ; rax clobbered
	mov	qword [rdi + 1*8], rbx
	mov	qword [rdi + 2*8], 0x1234567 ; rcx clobbered
	mov	qword [rdi + 3*8], r10
	mov	qword [rdi + 4*8], rsi
	mov	qword [rdi + 5*8], rax
	mov	qword [rdi + 6*8], rcx
	mov	qword [rdi + 7*8], rbp
	mov	qword [rdi + 8*8], r8
	mov	qword [rdi + 9*8], r9
	mov	qword [rdi + 10*8], 0x1234567 ; r10 clobbered
	mov	qword [rdi + 11*8], 0x1234567 ; r11 clobbered
	mov	qword [rdi + 12*8], 0x1234567 ; r12 clobbered
	mov	qword [rdi + 13*8], r13
	mov	qword [rdi + 14*8], r14
	mov	qword [rdi + 15*8], r15

	; rflags
	mov	qword [rdi + 16*8], r11
	; rip
	mov	qword [rdi + 17*8], r12

	; segment registers
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

	; no need to change saved cr3

	; clear "in use" flag
	btr	qword [rdi + 27*8], 5
.skip_save:
	; in case we took the shortcut, conditional-move task #0 into rdi
	mov	rax, task_state_region
	cmove	rdi, rax


	; find current index
	sub	rdi, task_state_region
	shr	rdi, 8

	mov	rcx, num_tasks
.search_loop:
	mov	rax, rdi
	shl	rax, 8
	add	rax, task_state_region

	; initial mask: VALID, RUNNABLE, BLOCKED, AP, and LOCKED flags
	mov	rbx, (1<<0) + (1<<1) + (1<<3) + (1<<4) + (1<<5)
	xor	rbx, qword [rax + 27*8]
	; want to see exactly BLOCKED | LOCKED ...
	cmp	rbx, (1<<3) + (1<<5)
	; if not, skip even trying to lock it
	jne	.skip_lock

	; try locking!
	lock bts qword [rax + 27*8], 5
	; if it was already locked, then we failed, so skip
	jc	.skip_lock

	; now that it's locked, ensure it's valid, runnable, not blocked, AP,
	; and not locked
	mov	rbx, (1<<0) + (1<<1) + (1<<3) + (1<<4) + (1<<5)
	xor	rbx, qword [rax + 27*8]
	; want to see exactly BLOCKED
	cmp	rbx, (1<<3)
	je	.found_task

	; something blocked it... release lock
	lock btr qword [rax + 27*8], 5

.skip_lock:
	inc	rdi
	and	rdi, (num_tasks - 1)
	loop	.search_loop

	; if we're here, there are no tasks available to schedule
	; wait for next timer interrupt

	.wait_loop:
	jmp	.wait_loop

.found_task:
	xor	rdi, rdi
	mov	rsi, rax
	mov	rax, transfer_control
	jmp	rax
