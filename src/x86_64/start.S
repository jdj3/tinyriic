        .globl _start
        .globl syscall
        
_start:
        pop %rdi
        mov %rsp, %rsi
        call main
        mov %rax, %rdi
        movq $60, %rax
        syscall

syscall:
        mov %rdi, %rax
        mov %rsi, %rdi
        mov %rdx, %rsi
        mov %rcx, %rdx
        mov %r8, %r10
        mov %r9, %r8
        movq 0x8(%rsp), %r9
        syscall
        ret
