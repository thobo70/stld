.section .text
.global _start

_start:
    movw $0x1234, %ax
    movw $0x5678, %bx
    addw %bx, %ax

.section .data
    message: .ascii "Hello!"
    number: .word 0x9999
