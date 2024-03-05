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
#include <string.h>
#include <time.h>
#include <threads.h>
#include <errno.h>

#include <hwpwm.h>

#include "offsets.h"
#include "gpio.h"
#include "stepper.h"
#include "joint.h"

#define NSTEPPERS 1
#define NJOINTS 6
#define SERVO_PWM_PERIOD 20000000UL
#define STEP_SLOW 30000000UL
#define STEP_MEDIUM 20000000UL
#define STEP_FAST 10000000UL

enum actuator_kind {
    ACTUATOR_STEPPER,
    ACTUATOR_SERVO,
};

struct stepper_joint {
    struct stepper *stepper;
    uint_least64_t delay;
};

struct servo_joint {
    unsigned channel;
    uint_fast16_t duty_cycle;
};

struct joint {
    union {
        struct stepper_joint stepper;
        struct servo_joint servo;
    } actuator;
    enum actuator_kind kind;
};

struct arm {
    struct joint joints[NJOINTS];
    thrd_t tid;
    mtx_t lock;
    int joint_idx;
    int dir;
};

static const unsigned stepper_pins[NSTEPPERS][NPOLES] = {
    { 0, 1, 2,  3 },
};

static const unsigned long stepper_delays[NSTEPPERS] = {
    STEP_MEDIUM,
};

static const char *pwmchip = "/sys/class/pwm/pwmchip0";

void arm_select_joint(struct arm *arm, int joint)
{
        mtx_lock(&arm->lock);
        arm->joint_idx = joint >= NJOINTS ? (NJOINTS - 1) : joint;
        printf("joint %d selected\n", joint);
        mtx_unlock(&arm->lock);
}

void arm_set_dir(struct arm *arm, int dir)
{
        mtx_lock(&arm->lock);
        arm->dir = dir;
        mtx_unlock(&arm->lock);
}

static void arm_joint_forward(struct arm *arm, int joint)
{
    if(arm->joints[joint].kind == ACTUATOR_STEPPER) {
        stepper_forward(arm->joints[joint].actuator.stepper.stepper);
        nanosleep(&(struct timespec) {.tv_nsec = arm->joints[joint].actuator.stepper.delay}, NULL);
    } else {
        printf("servo %d forward\n", joint - 1);
        struct servo_joint *s = &arm->joints[joint].actuator.servo;
        if(s->duty_cycle < 100)
            s->duty_cycle += 2;
        hwpwm_channel_set_duty_cycle_percent(pwmchip, s->channel, s->duty_cycle);
        if(errno) {
            fprintf(stderr, "hwpwm_channel_set_duty_cycle() failed: %s\n", strerror(errno));
        }
    }
}

static void arm_joint_backward(struct arm *arm, int joint)
{
    if(arm->joints[joint].kind == ACTUATOR_STEPPER) {
        stepper_backward(arm->joints[joint].actuator.stepper.stepper);
        nanosleep(&(struct timespec) {.tv_nsec = arm->joints[joint].actuator.stepper.delay}, NULL);
    } else {
        printf("servo %d backward\n", joint);
        struct servo_joint *s = &arm->joints[joint].actuator.servo;
        if(s->duty_cycle > 0)
            s->duty_cycle -= 2;
        hwpwm_channel_set_duty_cycle_percent(pwmchip, s->channel, s->duty_cycle);
        if(errno) {
            fprintf(stderr, "hwpwm_channel_set_duty_cycle() failed: %s\n", strerror(errno));
        }
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
        // second and third steppers move at the same time
        switch(dir) {
            case JOINT_STILL:
                break;
            case JOINT_BACKWARD:
                arm_joint_backward(arm, joint);
                break;
            case JOINT_FORWARD:
                arm_joint_forward(arm, joint);
                break;
        }
    }

    return 0;
}

struct arm *arm_init(void)
{
    int ret;
    struct arm *arm = malloc(sizeof(struct arm));
    for(int i = 0; i < NJOINTS; i++) {
        if(i == 0) {
            arm->joints[i].kind = ACTUATOR_STEPPER;
            struct stepper_joint *j = &arm->joints[i].actuator.stepper;
            j->delay = stepper_delays[i];
            j->stepper = stepper_init(chip,
                                      stepper_pins[i]);
        } else {
            arm->joints[i].kind = ACTUATOR_SERVO;
            int chidx = i - 1;
            struct servo_joint *s = &arm->joints[i].actuator.servo;
            if((ret = hwpwm_chip_export(pwmchip, chidx)) < 0)
                fprintf(stderr, "hwpwm_chip_export(): %s\n", strerror(hwpwm_error(ret)));
            s->channel = chidx;
            if((ret = hwpwm_channel_set_polarity(pwmchip, s->channel, HWPWM_POLARITY_NORMAL)) < 0)
                fprintf(stderr, "hwpwm_channel_set_polarity(): %s\n", strerror(hwpwm_error(ret)));
            if((ret = hwpwm_channel_set_period(pwmchip, s->channel, SERVO_PWM_PERIOD)) < 0)
                fprintf(stderr, "hwpwm_channel_set_period(): %s\n", strerror(hwpwm_error(ret)));
            s->duty_cycle = 0;
            if((ret = hwpwm_channel_set_duty_cycle_percent(pwmchip, s->channel, s->duty_cycle)) < 0)
                fprintf(stderr, "hwpwm_channel_set_duty_cycle_percent(): %s\n", strerror(hwpwm_error(ret)));
        }
    }
    arm->joint_idx = 0;
    arm->dir = JOINT_STILL;

    mtx_init(&arm->lock, mtx_plain);
    thrd_create(&arm->tid, arm_thread, arm);

    return arm;
}
