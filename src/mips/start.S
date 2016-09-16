        .globl __start
        .globl syscall
        
__start: .ent __start
        .frame $sp, 32, $ra
        .set noreorder
        .cpload $25
        .set reorder
        addiu $sp, $sp, -0x20
        .cprestore 8
        lw $a0, 0x20($sp)
        addiu $a1, $sp, 0x24
        jal main
        move $a0, $v0
        ori $v0, $zero, 4001
        syscall

        .end __start

syscall:
        addiu $sp, $sp, -0x18
        move $v0, $a0
        move $a0, $a1
        move $a1, $a2
        move $a2, $a3
        lw $a3, 0x2c($sp)
        nop
        sw $a3, 0x10($sp)
        lw $a3, 0x30($sp)
        nop
        sw $a3, 0x14($sp)
        lw $a3, 0x28($sp)
        nop
        syscall
        addiu $sp, $sp, 0x18
        jr $ra
