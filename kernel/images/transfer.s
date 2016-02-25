[BITS 64]
[ORG 0xfffffffffff0000]

; Expected as input:
; 	rax: points to task state structure
transfer_control:
	jmp	transfer_control
