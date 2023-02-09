#include <stdlib.h>

#include "thread.h"

struct command_queue {
    enum command *cmds;
    int cmds_size;
    int cmds_count;
};

static struct command_queue *command_queue_new(void)
{
    struct command_queue *queue = malloc(sizeof(struct command_queue));
    queue->cmds_size = 32; //kokemeny 128 byte per queue, oh a horror
    queue->cmds_count = 0;
    queue->cmds = malloc(sizeof(enum command) * queue->cmds_size);

    return queue;

}

void thread_command(struct thread *t, enum command cmd)
{
    
}

enum command command_queue_pop(struct command_queue *queue)
{
}
