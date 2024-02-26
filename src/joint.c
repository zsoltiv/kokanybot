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
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <threads.h>

#include "offsets.h"
#include "gpio.h"
#include "stepper.h"
#include "joint.h"

#define NSTEPPERS 5
#define STEP_SLOW 30000000
#define STEP_MEDIUM 20000000
#define STEP_FAST 10000000

struct joint {
    struct stepper *stepper;
    uint_least64_t delay;
    bool direction;
};

struct arm {
    struct joint joints[NSTEPPERS];
    thrd_t tid;
    mtx_t lock;
    int joint_idx;
    int dir;
};

static const unsigned stepper_pins[NSTEPPERS][NPOLES] = {
    {  0,   1,  2,  3 },
    {  4,   5,  6,  7 },
    {  8,   9, 10, 11 },
    { 12,  13, 14, 15 },
    {  0,   1,  2,  3 },
};

static const unsigned long stepper_delays[NSTEPPERS] = {
    STEP_SLOW,
    STEP_MEDIUM,
    STEP_MEDIUM,
    STEP_SLOW,
    STEP_SLOW,
};

void arm_select_joint(struct arm *arm, int joint)
{
        mtx_lock(&arm->lock);
        arm->joint_idx = joint;
        mtx_unlock(&arm->lock);
}

void arm_set_dir(struct arm *arm, int dir)
{
        mtx_lock(&arm->lock);
        arm->dir = dir;
        mtx_unlock(&arm->lock);
}

static int arm_thread(void *arg)
{
    struct arm *arm = (struct arm *)arg;

    while(true) {
        mtx_lock(&arm->lock);
        int dir = arm->dir;
        int joint = arm->joint_idx;
        mtx_unlock(&arm->lock);
        // second and third steppers move at the same time
        if(joint == 1 || joint == 2) {
            int other = joint == 1 ? 2 : 1;
            switch(dir) {
                case JOINT_STILL:
                    break;
                case JOINT_BACKWARD:
                    stepper_backward(arm->joints[joint].stepper);
                    stepper_forward(arm->joints[other].stepper);
                    break;
                case JOINT_FORWARD:
                    stepper_forward(arm->joints[joint].stepper);
                    stepper_backward(arm->joints[other].stepper);
                    break;
            }
        } else {
            switch(dir) {
                case JOINT_STILL:
                    break;
                case JOINT_BACKWARD:
                    stepper_backward(arm->joints[joint].stepper);
                    break;
                case JOINT_FORWARD:
                    stepper_forward(arm->joints[joint].stepper);
                    break;
            }
        }
        nanosleep(&(struct timespec) {.tv_nsec = arm->joints[joint].delay}, NULL);
    }

    return 0;
}

struct arm *arm_init(void)
{
    stepper_chips[0] = gpiod_chip_open("/dev/kokanystepperctl0");
    if(!stepper_chips[0])
        perror("gpiod_chip_open");
    stepper_chips[1] = gpiod_chip_open("/dev/kokanystepperctl1");
    if(!stepper_chips[1])
        perror("gpiod_chip_open");
    struct arm *arm = malloc(sizeof(struct arm));
    for(int i = 0; i < NSTEPPERS; i++) {
        arm->joints[i].delay = stepper_delays[i];
        arm->joints[i].stepper = stepper_init(stepper_chips[i == STEPPERSPERCHIP],
                                              stepper_pins[i]);
    }
    arm->joint_idx = 0;
    arm->dir = JOINT_STILL;

    mtx_init(&arm->lock, mtx_plain);
    thrd_create(&arm->tid, arm_thread, arm);

    return arm;
}
