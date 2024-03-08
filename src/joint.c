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
#include <inttypes.h>

#include <hwpwm.h>

#include "offsets.h"
#include "gpio.h"
#include "stepper.h"
#include "joint.h"

#define NJOINTS 3
#define MG996_PERIOD 20000000ULL
#define DUTY_CYCLE_PERCENT_STEP 0.2

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
    double duty_cycle_percent;
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

static const bool servo_invert_directions[NJOINTS] = {
    false,
    true,
    false,
};

static const double servo_default_duty_cycles[NJOINTS] = {
    71.f,
    28.f,
    50.f,
};

static const struct servo_limit {
    double min, max;
} servo_limits[NJOINTS] = {
    {  0.f, 100.f },
    {  0.f, 100.f },
    { 25.f, 100.f },
};

static const char *pwmchip = "/sys/class/pwm/pwmchip0";

void arm_select_joint(struct arm *arm, int joint)
{
        mtx_lock(&arm->lock);
        int j = 15 - joint;
        arm->joint_idx = j >= NJOINTS ? (NJOINTS - 1) : j;
        printf("joint %d selected\n", joint);
        mtx_unlock(&arm->lock);
}

static inline int maybe_invert(struct arm *arm, int idx, int dir)
{
    if(servo_invert_directions[idx]) {
        switch(dir) {
            case JOINT_FORWARD:
                return JOINT_BACKWARD;
            case JOINT_BACKWARD:
                return JOINT_FORWARD;
            default:
                return JOINT_STILL;
        }
    } else
        return dir;
}

void arm_set_dir(struct arm *arm, int dir)
{
        mtx_lock(&arm->lock);
        arm->dir = dir;
        mtx_unlock(&arm->lock);
}

static inline void arm_sleep(void)
{
    nanosleep(&(struct timespec){.tv_sec=0,.tv_nsec=10000000ULL},NULL);
}

static void arm_joint_forward(struct arm *arm, int joint)
{
    if(arm->joints[joint].kind == ACTUATOR_STEPPER) {
        stepper_forward(arm->joints[joint].actuator.stepper.stepper);
        nanosleep(&(struct timespec) {.tv_sec=0,.tv_nsec=arm->joints[joint].actuator.stepper.delay}, NULL);
    } else {
        struct servo_joint *s = &arm->joints[joint].actuator.servo;
        if(s->duty_cycle_percent < servo_limits[joint].max) {
            s->duty_cycle_percent = s->duty_cycle_percent + DUTY_CYCLE_PERCENT_STEP;
            printf("duty_cycle_percent=%f\n", s->duty_cycle_percent);
            int ret;
            if((ret = hwpwm_channel_set_duty_cycle_percent(pwmchip, s->channel, s->duty_cycle_percent / 10)) < 0)
                fprintf(stderr, "hwpwm_channel_set_duty_cycle() failed: %s\n", strerror(errno));
        }
    }
}

static void arm_joint_backward(struct arm *arm, int joint)
{
    int ret;
    if(arm->joints[joint].kind == ACTUATOR_STEPPER) {
        stepper_backward(arm->joints[joint].actuator.stepper.stepper);
        nanosleep(&(struct timespec) {.tv_sec=0,.tv_nsec=arm->joints[joint].actuator.stepper.delay}, NULL);
    } else {
        printf("servo %d backward\n", joint);
        struct servo_joint *s = &arm->joints[joint].actuator.servo;
        if(s->duty_cycle_percent > servo_limits[joint].min) {
            s->duty_cycle_percent = s->duty_cycle_percent - DUTY_CYCLE_PERCENT_STEP;
            printf("duty_cycle_percent=%f\n", s->duty_cycle_percent);
            if((ret = hwpwm_channel_set_duty_cycle_percent(pwmchip, s->channel, s->duty_cycle_percent / 10)) < 0)
                fprintf(stderr, "hwpwm_channel_set_duty_cycle() failed: %s\n", strerror(hwpwm_error(ret)));
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
        switch(maybe_invert(arm, joint, dir)) {
            case JOINT_STILL:
                break;
            case JOINT_BACKWARD:
                arm_joint_backward(arm, joint);
                break;
            case JOINT_FORWARD:
                arm_joint_forward(arm, joint);
                break;
        }
        arm_sleep();
    }

    return 0;
}

struct arm *arm_init(void)
{
    int ret;
    struct arm *arm = malloc(sizeof(struct arm));
    uint64_t npwm = hwpwm_chip_npwm(pwmchip);
    printf("npwm=%"PRIu64"\n", npwm);
    for(int i = 0; i < npwm; i++)
        if((ret = hwpwm_chip_unexport(pwmchip, i)) < 0)
            fprintf(stderr, "hwpwm_chip_unexport(): %s\n", strerror(hwpwm_error(ret)));
    for(int i = 0; i < NJOINTS; i++) {
        arm->joints[i].kind = ACTUATOR_SERVO;
        struct servo_joint *s = &arm->joints[i].actuator.servo;
        s->channel = npwm - i - 2;
        printf("chan=%u\n", s->channel);
        if((ret = hwpwm_chip_export(pwmchip, s->channel)) < 0)
            fprintf(stderr, "hwpwm_chip_export(): %s\n", strerror(hwpwm_error(ret)));
        if((ret = hwpwm_channel_set_enable(pwmchip, s->channel, false)) < 0)
            fprintf(stderr, "hwpwm_channel_set_enable(): %s\n", strerror(hwpwm_error(ret)));
        if((ret = hwpwm_channel_set_period(pwmchip,
                                           s->channel,
                                           MG996_PERIOD)) < 0)
            fprintf(stderr, "hwpwm_channel_set_period(): %s\n", strerror(hwpwm_error(ret)));
        if((ret = hwpwm_channel_set_polarity(pwmchip, s->channel, HWPWM_POLARITY_NORMAL)) < 0)
            fprintf(stderr, "hwpwm_channel_set_polarity(): %s\n", strerror(hwpwm_error(ret)));
        s->duty_cycle_percent = servo_default_duty_cycles[i];
        if((ret = hwpwm_channel_set_duty_cycle_percent(pwmchip,
                                                       s->channel,
                                                       s->duty_cycle_percent / 10)) < 0)
            fprintf(stderr, "hwpwm_channel_set_duty_cycle_percent(): %s\n", strerror(hwpwm_error(ret)));
        if((ret = hwpwm_channel_set_enable(pwmchip, s->channel, true)) < 0)
            fprintf(stderr, "hwpwm_channel_set_enable(): %s\n", strerror(hwpwm_error(ret)));
    }
    arm->joint_idx = 0;
    arm->dir = JOINT_STILL;

    mtx_init(&arm->lock, mtx_plain);
    thrd_create(&arm->tid, arm_thread, arm);

    return arm;
}
