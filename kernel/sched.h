#ifndef _DEF_SCHED
#define _DEF_SCHED

#include "type.h"

typedef enum {
    thread_running,
    thread_waiting,
} thread_state_t;

// thread context
typedef struct {
    uint64_t x19;
    uint64_t x20;
    uint64_t x21;
    uint64_t x22;
    uint64_t x23;
    uint64_t x24;
    uint64_t x25;
    uint64_t x26;
    uint64_t x27;
    uint64_t x28;
    uint64_t fp;    // x29
    uint64_t lr;    // x30
    uint64_t sp;
} thread_ctx_t;

typedef struct {
    thread_state_t state;
    thread_ctx_t ctx;

    void *user_sp;
    void *kernel_sp;
} thread_t;

void init_sched(void);
void create_thread(void *code);


thread_t *current_thread;
void set_current_thread(thread_t *thread);
static inline void set_current_ctx(thread_ctx_t *ctx) {
    __asm__ __volatile__( "msr tpidr_el1, %0" : : "r"(ctx));
}

#endif
