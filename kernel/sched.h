#ifndef _DEF_SCHED
#define _DEF_SCHED

// #include "list.h"
#include "type.h"
#include "syscall.h"

typedef enum {
    thread_running,
    thread_waiting,
    thread_dead,
} thread_state_t;

// thread context
// context switch 時，prev thread 的 ctx 會被儲存起來
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

typedef struct thread {
    int tid;

    thread_state_t state;
    thread_ctx_t ctx;

    /**
     * A thread can not only execute a piece of code in kernel,
     * but also execute a program.
     * 
     * If it's a piece of code, codesize = 0,
     * else, codesize = sizeof(program).
    */
    void *code;
    int codesize;

    void *user_sp;
    void *kernel_sp;

    struct thread *prev;
    struct thread *next;
} thread_t;

#define THREAD_STACK_SIZE 0x1000

void init_sched(void);
thread_t *create_thread(void *code, int codesize);
void schedule(void);
void _exit(void);
void kill_zombies();
int _exec(const char *name, char *const argv[]);
int _fork(trapframe_t *trapframe);
void _kill(int pid);

thread_t *get_current_thread(void);
void set_current_thread(thread_t *thread);
static inline void set_current_ctx(thread_ctx_t *ctx) {
    __asm__ __volatile__( "msr tpidr_el1, %0" : : "r"(ctx));
}

void thread_list_add(thread_t *item);

void idle(void);
void idle2(void);

#endif
