#include <stdbool.h>
#include <stdlib.h>
#include <threads.h>

#include <gpiod.h>

#include "gpio.h"
#include "mq135.h"

struct mq135 {
    bool gas_present;
    mtx_t lock;
    struct gpiod_line *line;
};

struct mq135 *mq135_init(unsigned int pin)
{
    struct mq135 *sensor = malloc(sizeof(struct mq135));
    sensor->gas_present = false;
    sensor->line = gpiod_chip_get_line(chip, pin);
    gpiod_line_request_both_edges_events(sensor->line, GPIO_CONSUMER);
    mtx_init(&sensor->lock, mtx_plain);

    return sensor;
}

void mq135_lock(struct mq135 *sensor)
{
    mtx_lock(&sensor->lock);
}

void mq135_unlock(struct mq135 *sensor)
{
    mtx_unlock(&sensor->lock);
}

bool mq135_get_presence(struct mq135 *sensor)
{
    return sensor->gas_present;
}

int mq135_thread(void *arg)
{
    struct mq135 *sensor = (struct mq135 *)arg;
    struct gpiod_line_event ev;

    while(true) {
        gpiod_line_event_wait(sensor->line, NULL);
        mtx_lock(&sensor->lock);
        gpiod_line_event_read(sensor->line, &ev);
        sensor->gas_present = ev.event_type == GPIOD_LINE_EVENT_FALLING_EDGE;
        mtx_unlock(&sensor->lock);
    }
    return 0;
}
