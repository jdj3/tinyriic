        .globl _start
        .globl syscall
        
_start:
        pop %eax
        push %esp
        push %eax
        call main
        mov %eax, %ebx
        movl $1, %eax
        int $0x80

syscall:
        push %ebx
        push %esi
        push %edi
        push %ebp
        movl 0x14(%esp), %eax
        movl 0x18(%esp), %ebx
        movl 0x1c(%esp), %ecx
        movl 0x20(%esp), %edx
        movl 0x24(%esp), %esi
        movl 0x28(%esp), %edi
        movl 0x2c(%esp), %ebp
        int $0x80
        pop %ebp
        pop %edi
        pop %esi
        pop %ebx
        ret
