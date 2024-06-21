#include "initramfs.h"
#include "malloc.h"
#include "mm.h"
#include "sched.h"
#include "shell.h"
#include "string.h"
#include "uart.h"

thread_t *thread_list;
thread_t *current_thread;
int max_tid;    // maximum tid that has been distributed

void init_sched(void)
{
    thread_list = NULL;
    max_tid = 0;
}

thread_t *create_thread(void *code, int codesize)
{
    thread_t *thread = (thread_t *) malloc(sizeof(thread_t));
    thread->tid = max_tid++;
    thread->state = thread_running;

    thread->next = NULL;
    thread->prev = NULL;

    thread->code = code;
    thread->codesize = codesize;

    thread->kernel_sp = malloc(THREAD_STACK_SIZE);
    thread->user_sp = malloc(THREAD_STACK_SIZE);

    thread->ctx.lr = code;
    thread->ctx.sp = thread->kernel_sp + THREAD_STACK_SIZE; // stack from high to low
    thread->ctx.fp = thread->kernel_sp + THREAD_STACK_SIZE;

    thread_list_add(thread);
    return thread;
}

void schedule()
{
    thread_t *prev = current_thread;
    thread_t *next;

    thread_t *cur = prev;

    // select a thread
    do {
        if (cur->next) {
            next = cur->next;
            cur = cur->next;
        } else {
            next = thread_list;
        }
    } while (next->state != thread_running);

    // uart_puts("schedule from ");
    // uart_putints(prev->tid);
    // uart_puts(" to ");
    // uart_putints(next->tid);
    // uart_puts("\n");

    set_current_thread(next);
    switch_to(&prev->ctx, &next->ctx);
}

void _exit(void)
{
    current_thread->state = thread_dead;
    schedule();
}

void kill_zombies()
{
    thread_t *cur = thread_list;
    while (cur) {
        if (cur->state == thread_dead) {
            // TODO: free stack
            free(cur->user_sp);
            free(cur->kernel_sp);
        }
        cur = cur->next;
    }
}

// Run different program in EL0
int _exec(const char *name, char *const argv[])
{
    thread_t *t = current_thread;

    cpio_meta_t *m = find_initramfs(name);
    t->code = buddy_allocate(m->filesize);
    memcpy(t->code, m->content, m->filesize);
    t->codesize = m->filesize;

    t->ctx.lr = t->code;

    asm("msr elr_el1, %0\t\n"
        "msr sp_el0, %1\t\n"
        "mov sp, %2\t\n" : : "r"(t->code), "r"(t->user_sp + THREAD_STACK_SIZE), "r"(t->kernel_sp + THREAD_STACK_SIZE));
    asm("mov x0, 0x000\t\n"
        "msr spsr_el1, x0\t\n");
    asm("eret\t\n");
}

/**
 * 
*/
int _fork(trapframe_t *trapframe)
{
    // critical section, cannot be interrupted
    thread_t *t = create_thread(current_thread->code, current_thread->codesize);
    thread_t *parent = current_thread;  // child would not execute this line

    // copy stack content from current_thread
    memcpy(t->kernel_sp, current_thread->kernel_sp, THREAD_STACK_SIZE);
    memcpy(t->user_sp, current_thread->user_sp, THREAD_STACK_SIZE);

    // set fp, sp register
    uint64_t fp, sp;
    asm("mov %0, fp\t\n" : "=r"(fp));
    asm("mov %0, sp\t\n" : "=r"(sp));

    uint64_t fp_offset, sp_offset;
    fp_offset = fp - (uint64_t) current_thread->kernel_sp;
    sp_offset = sp - (uint64_t) current_thread->kernel_sp;

    t->ctx.fp = t->kernel_sp + fp_offset;
    t->ctx.sp = t->kernel_sp + fp_offset;

    // set lr register
    asm("adr %0, ." : "=r"(t->ctx.lr)); // go back to here after context switch
    // t->ctx.lr += 4 * 8;

    // copy code
        // No, because share code, only exec would load new code

    // after fork，parent and child (kernel mode) would go here
    if (current_thread->tid == t->tid) {    // child
        trapframe = t->kernel_sp + (uint64_t) trapframe - (uint64_t) parent->kernel_sp;
        trapframe->x0 = 0;
        return 0;
    }
    // return t->tid;
    trapframe->x0 = t->tid;
}

void _kill(int pid)
{
    thread_t *cur = thread_list;
    while (cur) {
        if (cur->tid == pid) {
            cur->state = thread_dead;
            return;
        }
    }
}

thread_t *get_current_thread(void)
{
    return current_thread;
}

void set_current_thread(thread_t *thread)
{
    current_thread = thread;
    set_current_ctx(&current_thread->ctx);
}

void thread_list_add(thread_t *item)
{
    if (thread_list == NULL) {
        thread_list = item;
        return;
    }
    item->next = thread_list;
    thread_list->prev = item;
    thread_list = item;
}
