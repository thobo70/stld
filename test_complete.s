.section .text
.global _start

_start:
    movw $0x1234, %ax
    movw $0x5678, %bx
    addw %bx, %ax
    jmp end_program

.section .data
    message: .ascii "Hello, World!"
    number: .word 0x9999

.section .bss
    buffer: .space 64

end_program:
    movw $0x4C00, %ax
    int $0x21
