        .globl _start
        .globl syscall
        
_start:
        pop {r0}
        mov r1, sp
        bl main
        mov r7, #1
        svc #0

syscall:
        push {r4, r5, r6, r7, lr}
        mov r7, r0
        mov r0, r1
        mov r1, r2
        mov r2, r3
        ldr r3, [sp, #0x14]
        ldr r4, [sp, #0x18]
        ldr r5, [sp, #0x1c]
        ldr r6, [sp, #0x20]
        svc #0
        pop {r4, r5, r6, r7, pc}
