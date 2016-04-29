[BITS 64]

[ORG 0xffffa00000000000]

%idefine sysretq o64 sysret

syscall:
	.loop:
	jmp .loop
	sysretq
