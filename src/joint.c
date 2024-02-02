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
#include "mcp23017.h"
#include "joint.h"

#define NSTEPPERS 6
#define STEP_SLOW 30000000
#define STEP_MEDIUM 20000000
#define STEP_FAST 10000000

struct arm {
    struct joint *joints[NSTEPPERS];
    struct mcp23017 *mcp;
    thrd_t tid;
    mtx_t lock;
    int joint_idx;
    int dir;
};

struct joint {
    struct stepper *stepper;
    uint_least64_t delay;
};

// WARN DO NOT USE YET

static const unsigned stepper_pins[NSTEPPERS][NPOLES] = {
    { GPIO26, GPIO19, GPIO13, GPIO6 },
    { GPIO5, SPI_SCLK, SPI_MISO, SPI_MOSI },
    { GPIO21, GPIO20, GPIO12, SPI_CE1_N },
    { SPI_CE0_N, ID_SDA, GPIO24, GPIO_GCLK },
};


static const unsigned long stepper_delays[NSTEPPERS] = {
    STEP_SLOW,
    STEP_MEDIUM,
    STEP_MEDIUM,
    STEP_SLOW,
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

static void joint_forward(struct arm *arm, int i)
{
    stepper_forward(arm->joints[i]->stepper);
}

static void joint_backward(struct arm *arm, int i)
{
    stepper_backward(arm->joints[i]->stepper);
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
                    joint_backward(arm, joint);
                    joint_forward(arm, other);
                    break;
                case JOINT_FORWARD:
                    joint_forward(arm, joint);
                    joint_backward(arm, other);
                    break;
            }
        } else {
            switch(dir) {
                case JOINT_STILL:
                    break;
                case JOINT_BACKWARD:
                    joint_backward(arm, joint);
                    break;
                case JOINT_FORWARD:
                    joint_forward(arm, joint);
                    break;
            }
        }
        nanosleep(&(struct timespec) {.tv_nsec = arm->joints[joint]->delay}, NULL);
    }

    return 0;
}

struct arm *arm_init(void)
{
    struct arm *arm = malloc(sizeof(struct arm));
    arm->mcp = mcp23017_init(0x20);
    for(int i = 0; i < NSTEPPERS; i++) {
        arm->joints[i] = malloc(sizeof(struct joint));
        arm->joints[i]->delay = stepper_delays[i];
        // first four steppers fit on the gpio pins
        arm->joints[i]->stepper = stepper_init(chip, stepper_pins[i]);
    }
    arm->joint_idx = 0;
    arm->dir = JOINT_STILL;

    mtx_init(&arm->lock, mtx_plain);
    thrd_create(&arm->tid, arm_thread, arm);

    return arm;
}
