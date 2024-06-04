#ifndef _DEF_TIMER
#define _DEF_TIMER

#include "malloc.h"

// typedef struct timer {
//     char *message;
//     int second;
//     struct timer *prev;
//     struct timer *next;
// } timer_t;

void add_timer(const char *line);
// timer_t *init_timer(char *msg, int second);
// void append_timer(timer_t *timer);
void update_timers(void);

// static timer_t *timer_head = NULL;
__attribute__((unused)) static char *messages[100];
__attribute__((unused)) static int seconds[100];
__attribute__((unused)) static int timer_index = 0;

#endif
