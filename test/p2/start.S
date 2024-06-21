.section .text.start
.global _start

_start:
    mov x15, 0
1:
    add x15, x15, 1
    mov x7, 0x5487
    //svc 1

    bl main
    // get pid
    //mov x8, 0
    //svc 0
    //add x9, x0, 10

    cmp x15, 1
    blt 1b
1:
    b 1b
