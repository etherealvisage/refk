[BITS 64]
section .text

extern current_task
extern select_next_task

global yield
global enter_task
yield:
	; grab address of current_task
	push	rax
	mov	rax, current_task
	mov	rax, qword [rax]

	; save general-purpose registers
	; missing: RAX
	mov	qword [rax + 1*8], rbx
	mov	qword [rax + 2*8], rcx
	mov	qword [rax + 3*8], rdx
	mov	qword [rax + 4*8], rsi
	mov	qword [rax + 5*8], rdi
	mov	qword [rax + 6*8], rbp
	; missing: RSP
	pushfq
	pop	rbx
	mov	qword [rax + 8*8], rbx
	mov	qword [rax + 9*8], yield_exit

	; assume segment registers are the same for now

	; save missing registers (RAX, RSP)
	mov	rbx, rsp
	add	rbx, 8 ; one value still pushed on stack
	mov	qword [rax + 7*8], rbx ; rsp
	pop	rbx
	mov	qword [rax], rbx ; rax

	; task-state saved. switch to task stack.
	mov	rsp, task_stack

	call	select_next_task

enter_task:
	; restore task state
	mov	rax, current_task
	mov	rax, qword [rax]

	; restore general-purpose registers
	mov	rcx, qword [rax + 2*8]
	mov	rdx, qword [rax + 3*8]
	mov	rsi, qword [rax + 4*8]
	mov	rdi, qword [rax + 5*8]
	mov	rbp, qword [rax + 6*8]
	; not restored yet: rax, rbx

	; stack push order: SS, RSP, RFLAGS, CS, RIP
	xor	rbx, rbx
	mov	bx, ss
	push	rbx ; ss

	mov	rbx, qword [rax + 7*8]
	push	rbx ; rsp

	mov	rbx, qword [rax + 8*8]
	push	rbx ; rflags

	xor	rbx, rbx
	mov	bx, cs
	push	rbx ; cs

	mov	rbx, qword [rax + 9*8]
	push	rbx ; rip

	; rax, rbx
	mov	rbx, qword [rax + 1*8]
	mov	rax, qword [rax]

	iretq

yield_exit:
	ret

section .bss
	resb	1024
task_stack:
