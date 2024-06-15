.section .text.start
.global _start

_start:
    mov x0, 0
1:
    add x0, x0, 1
    //bl main
    mov x7, 0x5487
    //svc 0
    cmp x0, 1
    blt 1b
1:
    b 1b
