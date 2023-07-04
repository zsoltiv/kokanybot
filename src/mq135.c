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
    struct gpiod_line *line;
    thrd_t tid;
    int client, port;
    bool gas_present;
};

struct mq135 *mq135_init(unsigned port, unsigned pin)
{
    struct mq135 *sensor = malloc(sizeof(struct mq135));
    sensor->gas_present = false;
    sensor->line = gpiod_chip_get_line(chip, pin);
    sensor->port = port;
    if(gpiod_line_request_both_edges_events(sensor->line, GPIO_CONSUMER) < 0)
        perror("gpiod_line_request_both_edges_events()");
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
    struct gpiod_line_event ev;

    sensor->client = net_accept(sensor->port);

    while(true) {
        int ret;
        if((ret = gpiod_line_event_wait(sensor->line, NULL)) < 1) {
            if(ret < 0)
                perror("gpiod_line_event_wait()");
            continue;
        }
        if(gpiod_line_event_read(sensor->line, &ev) < 0)
            perror("gpiod_line_event_read()");
        sensor->gas_present = ev.event_type == GPIOD_LINE_EVENT_FALLING_EDGE;
        printf("tick %s\n", sensor->gas_present ? "GAS" : "NOGAS");
        if(send(sensor->client, &sensor->gas_present, 1, 0) < 0) {
            if(errno == ECONNRESET) {
                close(sensor->client);
                sensor->client = net_accept(sensor->port);
            }
            perror("send()");
        }
    }
    return 0;
}
