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

#include <stdio.h>
#include <stdbool.h>
#include <errno.h>
#include <stdint.h>
#include <stdlib.h>
#include <threads.h>
#include <time.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

#include "offsets.h"
#include "net.h"
#include "gpio.h"
#include "i2c.h"
#include "motor.h"
#include "mq135.h"
#include "input.h"

#define PORT_CTL 1337
#define PORT_SENSOR 1339

struct key_bind key_binds[UINT8_MAX] = {
    ['w'] = { .func =  motor_forward },
    ['a'] = { .func =     motor_left },
    ['s'] = { .func = motor_backward },
    ['d'] = { .func =    motor_right },
};

int main(void)
{
    input_init();
    gpio_init();
    motor_init();
    i2c_init();
    struct mq135 *mq135 = mq135_init(PORT_SENSOR, GPIO16);
    int client = net_accept(PORT_CTL);
    printf("Up and running\n");
    while(1) {
        uint8_t keycode = net_receive_keypress(client);
        if(!keycode && errno == ECONNRESET)
            client = net_accept(PORT_CTL);
        input_process_key_event(keycode);
    }
}
