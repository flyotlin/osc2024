.global getpid
getpid:
    mov x8, 0
    svc 0
    ret

.global uartread
uartread:
    mov x8, 1
    svc 0
    ret

.global uartwrite
uartwrite:
    mov x8, 2
    svc 0
    ret

.global exec
exec:
    mov x8, 3
    svc 0
    ret

.global fork
fork:
    mov x8, 4
    svc 0
    ret