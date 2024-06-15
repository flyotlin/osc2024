#include "sched.h"
#include "syscall.h"

void syscall_handler(trapframe_t *trapframe_t)
{
    int syscall_number = trapframe_t->x8;

    if (syscall_number == 0) {
        trapframe_t->x0 = getpid();
    }
}

int getpid(void)
{
    thread_t *cur = get_current_thread();
    return cur->tid;
}