#ifndef _DEF_SYSCALL
#define _DEF_SYSCALL

#include "type.h"

void syscall_handler(trapframe_t *trapframe);
int getpid();
int uart_read(char buf[], int size);
int uart_write(const char buf[], int size);
int exec(const char *name, char *const argv[]);
int fork(trapframe_t *trapframe);
int exit(void);
int mbox_call(trapframe_t *trapframe, unsigned char ch, unsigned int *mbox);
void kill(int pid);

#endif
