#include "sched.h"
#include "shell.h"
#include "malloc.h"
#include "uart.h"

thread_t *thread_list;
thread_t *current_thread;
int max_tid;    // maximum tid that has been distributed

void init_sched(void)
{
    thread_list = NULL;
    max_tid = 0;
}

thread_t *create_thread(void *code)
{
    thread_t *thread = (thread_t *) malloc(sizeof(thread_t));
    thread->tid = max_tid++;
    thread->state = thread_running;

    thread->next = NULL;
    thread->prev = NULL;

    thread->code = code;
    thread->codesize = 0;

    thread->kernel_sp = malloc(THREAD_STACK_SIZE);
    thread->user_sp = malloc(THREAD_STACK_SIZE);

    thread->ctx.lr = code;
    thread->ctx.sp = thread->kernel_sp + THREAD_STACK_SIZE; // stack from high to low
    thread->ctx.fp = thread->kernel_sp;

    thread_list_add(thread);
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

    set_current_thread(next);
    switch_to(&prev->ctx, &next->ctx);
}

int fork(void)
{
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
