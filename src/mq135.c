/*
 * copyright (c) 2023 Zsolt Vadasz
 *
 * This file is part of kokanybot.
 *
 * kokanybot is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License,
 * or (at your option) any later version.
 *
 * kokanybot is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with kokanybot. If not, see <https://www.gnu.org/licenses/>. 
*/

#define _XOPEN_SOURCE 700
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <threads.h>
#include <sys/types.h>

#include <gpiod.h>

#include "gpio.h"
#include "net.h"
#include "mq135.h"

struct mq135 {
    struct gpiod_line_request *line;
    struct gpiod_edge_event_buffer *events;
    thrd_t tid;
    int client, port;
    bool gas_present;
};

struct mq135 *mq135_init(unsigned port, unsigned pin)
{
    struct mq135 *sensor = malloc(sizeof(struct mq135));
    sensor->gas_present = false;
    sensor->line = gpio_init_input_events(pin, GPIOD_LINE_EDGE_BOTH);
    sensor->port = port;
    thrd_create(&sensor->tid, mq135_thread, sensor);

    return sensor;
}

bool mq135_get_presence(struct mq135 *sensor)
{
    return sensor->gas_present;
}

int mq135_thread(void *arg)
{
    struct mq135 *sensor = (struct mq135 *)arg;
    sensor->events = gpiod_edge_event_buffer_new(1);

    sensor->client = net_accept(sensor->port);

    while(true) {
        int ret;
        if(gpiod_line_request_read_edge_events(sensor->line, sensor->events, 1) < 0)
            perror("gpiod_line_event_read()");
        struct gpiod_edge_event *ev = gpiod_edge_event_buffer_get_event(sensor->events, 0);
        sensor->gas_present = gpiod_edge_event_get_event_type(ev) == GPIOD_EDGE_EVENT_FALLING_EDGE;
        printf("tick %s\n", sensor->gas_present ? "GAS" : "NOGAS");
        if(send(sensor->client, &sensor->gas_present, 1, 0) < 0) {
            if(errno == ECONNRESET || errno == EPIPE) {
                close(sensor->client);
                sensor->client = net_accept(sensor->port);
            }
            perror("send()");
        }
    }
    return 0;
}
