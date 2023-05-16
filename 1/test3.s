.intel_syntax noprefix
.globl plus, main

plus:
	add rdi, rsi
	mov rax, rdi
	ret

main:
	mov rdi, 3
	mov rsi, 4
	call plus
	ret
