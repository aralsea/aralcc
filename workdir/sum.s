.intel_syntax noprefix
.globl main
main:
    push rbp
    mov rbp, rsp
    sub rsp, 0
    push 2
    pop rdi
    push 3
    pop rsi
    call bar
    push rax
    pop rax
    mov rsp, rbp
    pop rbp
    ret
