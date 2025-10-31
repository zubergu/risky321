.section .text

.globl usys_fork
usys_fork:
    li a7, 1
    ecall
    ret

.globl usys_exit
usys_exit:
    li a7, 2
    ecall
    ret


.globl usys_read
usys_read:
    li a7, 5
    ecall
    ret


.globl usys_kill
usys_kill:
    li a7, 6
    ecall
    ret

.globl usys_exec
usys_exec:
    li a7, 7
    ecall
    ret

.globl usys_getpid
usys_getpid:
    li a7, 11
    ecall
    ret

.globl usys_sbrk
usys_sbrk:
    li a7, 12
    ecall
    ret

.globl usys_sleep
usys_sleep:
    li a7, 13
    ecall
    ret


.globl usys_open
usys_open:
    li a7, 15
    ecall
    ret


.globl usys_write
usys_write:
    li a7, 16
    ecall
    ret


.globl usys_close
usys_close:
    li a7, 21
    ecall
    ret
