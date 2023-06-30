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
#include "joint.h"
#include "motor.h"
#include "mq135.h"
#include "input.h"

#define PORT_CTL 1337
#define PORT_SENSOR 1339

static struct arm *arm;
static int joint_idx = 1;

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

#define STEPPER_SELECT_FUNC(n) static void stepper_select_##n(bool pressed)\
{\
    if(pressed)\
        arm_select_joint(arm, (n) - 1);\
}

#define STEPPER_BINDING(n) [(n) + '0'] = { .func = stepper_select_##n }

STEPPER_SELECT_FUNC(1)
STEPPER_SELECT_FUNC(2)
STEPPER_SELECT_FUNC(3)
STEPPER_SELECT_FUNC(4)
STEPPER_SELECT_FUNC(5)
STEPPER_SELECT_FUNC(6)
STEPPER_SELECT_FUNC(7)
STEPPER_SELECT_FUNC(8)

struct key_bind key_binds[UINT8_MAX] = {
    ['w'] = { .func =      motor_forward },
    ['a'] = { .func =         motor_left },
    ['s'] = { .func =     motor_backward },
    ['d'] = { .func =        motor_right },
    ['q'] = { .func = joint_sel_backward },
    ['e'] = { .func =  joint_sel_forward },
    STEPPER_BINDING(1),
    STEPPER_BINDING(2),
    STEPPER_BINDING(3),
    STEPPER_BINDING(4),
    STEPPER_BINDING(5),
    STEPPER_BINDING(6),
    STEPPER_BINDING(7),
    STEPPER_BINDING(8),
};

int main(void)
{
    input_init();
    gpio_init();
    motor_init();
    i2c_init();
    struct mq135 *mq135 = mq135_init(PORT_SENSOR, GPIO16);
    int client = net_accept(PORT_CTL);
    arm = arm_init();
    printf("Up and running\n");
    while(1) {
        uint8_t keycode = net_receive_keypress(client);
        if(!keycode && errno == ECONNRESET)
            client = net_accept(PORT_CTL);
        input_process_key_event(keycode);
    }
}
