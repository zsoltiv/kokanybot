#ifndef THREAD_H
#define THREAD_H

#ifdef __STDC_NO_THREADS__
#error "threads.h not supported"
#endif

#include <threads.h>

typedef int (*thread_func)(void *);

enum command {
    CMD_NONE,
    CMD_FORWARD,
    CMD_BACKWARD,
    CMD_TURN_LEFT,
    CMD_TURN_RIGHT,
    CMD_DO_IMAGE_RECOGNITION,
};

union thread_data {
    int degrees;
};

struct command_queue;

void thread_command(struct thread *t, enum command cmd);
enum command command_queue_pop(struct command_queue *queue);

struct thread {
    thrd_t tid;
    struct command_queue *queue;
    union thread_data td;
    mtx_t lock;
};

struct thread *thread_new(thread_func tf);

#endif /* THREAD_H */
