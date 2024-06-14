#include "sched.h"
#include "malloc.h"
#include "uart.h"

thread_t *thread_list;
thread_t *current_thread;
int max_tid = 0;

void init_sched(void)
{
    thread_list = NULL;

    create_thread(idle);

    for (int i = 0; i < 10; i++) {
        uart_puts("hi 1\n");
    }
    schedule();
}

thread_t *create_thread(void *code)
{
    thread_t *thread = (thread_t *) malloc(sizeof(thread_t));
    thread->tid = max_tid++;
    thread->state = thread_running;

    thread->next = NULL;
    thread->prev = NULL;

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

int forked = 0;
void idle()
{
    while (1) {
        for (int i = 0; i < 10; i++) {
            uart_puts("hi 2\n");
        }
        if (forked == 0) {
            forked = 1;
            create_thread(idle2);
        }
        schedule();
    }
}

void idle2()
{
    while (1) {
        for (int i = 0; i < 5; i++) {
            uart_puts("hi 3\n");
        }
        schedule();
    }
}
