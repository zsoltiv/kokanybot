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
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <threads.h>

#include "offsets.h"
#include "gpio.h"
#include "stepper.h"
#include "mcp23017.h"
#include "joint.h"

#define NSTEPPERS 8
#define STEP_NSEC 5000000

struct arm {
    struct joint *joints[NSTEPPERS];
    struct mcp23017 *mcp;
    thrd_t tid;
    mtx_t lock;
    int joint_idx;
    int dir;
};

struct joint {
    union {
        struct stepper *stepper;
        unsigned *mcp23017_stepper;
    } stepper;
    int step_idx;
    bool is_gpio;
};

static unsigned stepper_pins[4][4] = {
    { GPIO26, GPIO19, GPIO13, GPIO6 },
    { GPIO5, SPI_SCLK, SPI_MISO, SPI_MOSI },
    { GPIO21, GPIO20, GPIO12, SPI_CE1_N },
    { SPI_CE0_N, GPIO18, RXD0, TXD0 },
};

static unsigned stepper_mcp_pins[4][4] = {
    {  0,  1,  2,  3 },
    {  4,  5,  6,  7 },
    {  8,  9, 10, 11 },
    { 12, 13, 14, 15 },
};

/* BLUE - PINK - YELLOW - ORANGE */
static const int steps[][NPOLES] = {
    { 1, 1, 0, 0 },
    { 0, 1, 1, 0 },
    { 0, 0, 1, 1 },
    { 1, 0, 0, 1 },
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
    if(arm->joints[i]->is_gpio)
        stepper_forward(arm->joints[i]->stepper.stepper);
    else {
        arm->joints[i]->step_idx++;
        if(arm->joints[i]->step_idx == 4)
            arm->joints[i]->step_idx = 0;
        for(int pin = 0; pin < 4; pin++)
            mcp23017_set(arm->mcp,
                         arm->joints[i]->stepper.mcp23017_stepper[pin],
                         steps[arm->joints[i]->step_idx][pin]);
    }
}

static void joint_backward(struct arm *arm, int i)
{
    if(arm->joints[i]->is_gpio)
        stepper_backward(arm->joints[i]->stepper.stepper);
    else {
        arm->joints[i]->step_idx--;
        if(arm->joints[i]->step_idx < 0)
            arm->joints[i]->step_idx = 3;
        for(int pin = 0; pin < 4; pin++)
            mcp23017_set(arm->mcp,
                         arm->joints[i]->stepper.mcp23017_stepper[pin],
                         steps[arm->joints[i]->step_idx][pin]);
    }
}

static int arm_thread(void *arg)
{
    struct arm *arm = (struct arm *)arg;

    while(true) {
        mtx_lock(&arm->lock);
        int dir = arm->dir;
        int joint = arm->joint_idx;
        mtx_unlock(&arm->lock);
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
        nanosleep(&(struct timespec) {.tv_nsec = STEP_NSEC}, NULL);
    }

    return 0;
}

struct arm *arm_init(void)
{
    struct arm *arm = malloc(sizeof(struct arm));
    arm->mcp = mcp23017_init(0x20);
    int gstepper = 0;
    int mstepper = 0;
    for(int i = 0; i < NSTEPPERS; i++) {
        arm->joints[i] = malloc(sizeof(struct joint));
        arm->joints[i]->step_idx = 0;
        // first five steppers fit on the gpio pins
        arm->joints[i]->is_gpio = i < 4;
        printf("STEPPER %d GPIO? %d\n", i, arm->joints[i]->is_gpio);
        if(arm->joints[i]->is_gpio) {
            arm->joints[i]->stepper.stepper = stepper_init(chip, stepper_pins[gstepper]);
            gstepper++;
        } else {
            arm->joints[i]->stepper.mcp23017_stepper = stepper_mcp_pins[mstepper];
        }
    }
    arm->joint_idx = 1;
    arm->dir = JOINT_STILL;

    mtx_init(&arm->lock, mtx_plain);
    thrd_create(&arm->tid, arm_thread, arm);

    return arm;
}
