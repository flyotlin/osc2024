#include "mailbox.h"
#include "sched.h"
#include "syscall.h"
#include "uart.h"

void syscall_handler(trapframe_t *trapframe)
{
    int syscall_number = trapframe->x8;
    // int ret;

    thread_t *cur = get_current_thread();
    // if (cur->tid == 1) {
    //     uart_puts("syscall number: ");
    //     uart_putints(syscall_number);
    //     uart_puts("\n");
    // }

    if (syscall_number == 0) {
        trapframe->x0 = getpid();
    } else if (syscall_number == 1) {
        trapframe->x0 = uart_read(trapframe->x0, trapframe->x1);
    } else if (syscall_number == 2) {
        trapframe->x0 = uart_write(trapframe->x0, trapframe->x1);
    } else if (syscall_number == 3) {
        trapframe->x0 = exec(trapframe->x0, trapframe->x1);
    } else if (syscall_number == 4) {
        // ret = fork(trapframe);
        fork(trapframe);
    } else if (syscall_number == 5) {
        trapframe->x0 = exit();
    } else if (syscall_number == 6) {
        // trapframe->x0 = mbox_call(trapframe->x0, trapframe->x1);
        mbox_call(trapframe, trapframe->x0, trapframe->x1);
    } else if (syscall_number == 7) {
        kill(trapframe->x0);
    }
    // trapframe->x0 = ret;
}

int getpid(void)
{
    thread_t *cur = get_current_thread();
    return cur->tid;
}

int uart_read(char buf[], int size)
{
    return _uart_read(buf, size);
}

int uart_write(const char buf[], int size)
{
    return _uart_write(buf, size);
}

int exec(const char *name, char *const argv[])
{
    return _exec(name, argv);
}

int fork(trapframe_t *trapframe)
{
    return _fork(trapframe);
}

int exit(void)
{
    _exit();
    return 0;
}

int mbox_call(trapframe_t *trapframe, unsigned char ch, unsigned int *mbox)
{
    // return mailbox_exec(ch, mbox);
    return _mbox_exec(trapframe, ch, mbox);
}

void kill(int pid)
{
    _kill(pid);
}
