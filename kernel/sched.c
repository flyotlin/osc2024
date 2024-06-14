#include "sched.h"
#include "malloc.h"

void init_sched(void)
{
    // 幫目前的 context 也建立一個 thread

    thread_t *now = (thread_t *) malloc(sizeof(thread_t));
    set_current_thread(now);
}

void create_thread(void *code)
{

}

void set_current_thread(thread_t *thread)
{
    current_thread = thread;
    set_current_ctx(&current_thread->ctx);
}