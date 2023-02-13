#ifndef THREAD_H
#define THREAD_H

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __STDC_NO_THREADS__
#error "threads.h not supported"
#endif

#include <threads.h>

#include "cmd.h"

typedef int (*thread_func)(void *);


union thread_data {
    int degrees;
};

struct command_queue;

struct thread {
    thrd_t tid;
    struct command_queue *queue;
    union thread_data td;
    mtx_t lock;
};

void thread_command(struct thread *t, enum command cmd);
enum command command_queue_pop(struct command_queue *queue);

struct thread *thread_new(thread_func tf);

#ifdef __cplusplus
}
#endif

#endif /* THREAD_H */
