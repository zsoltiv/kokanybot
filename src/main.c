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
#include "joint.h"
#include "motor.h"
#include "mq135.h"
#include "input.h"

#define PORT_CTL 1337
#define PORT_SENSOR 1339

static struct arm *arm;

static void joint_sel_forward(bool pressed)
{
    if(pressed)
        arm_set_dir(arm, JOINT_FORWARD);
    else
        arm_set_dir(arm, JOINT_STILL);
}

static void joint_sel_backward(bool pressed)
{
    if(pressed)
        arm_set_dir(arm, JOINT_BACKWARD);
    else
        arm_set_dir(arm, JOINT_STILL);
}

#define JOINT_SELECT_FUNC(n) static void stepper_select_##n(bool pressed)\
{\
    if(pressed)\
        arm_select_joint(arm, 16 - (n));\
}

#define JOINT_BINDING(n) [(n) + '0'] = { .func = stepper_select_##n }

JOINT_SELECT_FUNC(1)
JOINT_SELECT_FUNC(2)
JOINT_SELECT_FUNC(3)
JOINT_SELECT_FUNC(4)
JOINT_SELECT_FUNC(5)
JOINT_SELECT_FUNC(6)
JOINT_SELECT_FUNC(7)
JOINT_SELECT_FUNC(8)
JOINT_SELECT_FUNC(9)

struct key_bind key_binds[INT8_MAX] = {
    ['w'] = { .func =      motor_forward },
    ['a'] = { .func =         motor_left },
    ['s'] = { .func =     motor_backward },
    ['d'] = { .func =        motor_right },
    ['q'] = { .func = joint_sel_backward },
    ['e'] = { .func =  joint_sel_forward },
    JOINT_BINDING(1),
    JOINT_BINDING(2),
    JOINT_BINDING(3),
    JOINT_BINDING(4),
    JOINT_BINDING(5),
    JOINT_BINDING(6),
    JOINT_BINDING(7),
    JOINT_BINDING(8),
    JOINT_BINDING(9),
};

int main(void)
{
    input_init();
    gpio_init();
    motor_init();
    struct mq135 *mq135 = mq135_init(PORT_SENSOR, GPIO26);
    int client = net_accept(PORT_CTL);
    arm = arm_init();
    printf("Up and running\n");
    while(1) {
        uint8_t keycode = net_receive_keypress(client);
        if(!keycode) {
            close(client);
            client = net_accept(PORT_CTL);
        }
        input_process_key_event(keycode);
    }
}
