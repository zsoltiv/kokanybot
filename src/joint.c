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
#include <assert.h>

#include <hwpwm.h>

#include "joint.h"

#define NJOINTS 5
#define MG996_PERIOD UINT64_C(20000000)
#define PRECISION UINT64_C(100)
#define DUTY_CYCLE_STEP UINT64_C(1)

#define ELEMS(x) (sizeof((x)) / sizeof((x)[0]))

struct servo_joint {
    uint64_t duty_cycle_percent;
    unsigned channel;
};

struct arm {
    struct servo_joint joints[NJOINTS];
    thrd_t tid;
    mtx_t lock;
    int joint_idx;
    int dir;
};

static const bool servo_invert_directions[NJOINTS] = {
    false,
    true,
    false,
    false,
    true,
};

static const uint64_t servo_default_duty_cycles[NJOINTS] = {
    50,
    28,
    50,
    50,
    50,
};

static const struct servo_limit {
    uint64_t min, max;
} servo_limits[NJOINTS] = {
    {   0, PRECISION },
    {   0, PRECISION },
    {   0, PRECISION },
    {   0, PRECISION },
    {   0, PRECISION },
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

static inline uint64_t calculate_duty_cycle(const uint64_t period,
                                            const uint64_t precision,
                                            const uint64_t value)
{
    // duty cycle must be between 1ms and 2ms
    // XXX everything is in ns
    //return period / precision * value / 20 + UINT64_C(1000000);
    return period / precision * value / 10 + UINT64_C(1000000);
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
    // sleep 20ms
    nanosleep(&(struct timespec){.tv_sec=0,.tv_nsec=MG996_PERIOD},NULL);
}

static void arm_joint_apply_duty_cycle(struct arm *arm, int joint)
{
    struct servo_joint *s = &arm->joints[joint];
    printf("duty_cycle_percent=%"PRIu64"\n", s->duty_cycle_percent);
    int ret;
    if((ret = hwpwm_channel_set_duty_cycle(pwmchip,
                                           s->channel,
                                           calculate_duty_cycle(MG996_PERIOD,
                                                                PRECISION,
                                                                s->duty_cycle_percent))) < 0)
        fprintf(stderr, "hwpwm_channel_set_duty_cycle() failed: %s\n", strerror(errno));
    printf("%"PRIu64"\n", s->duty_cycle_percent);
}

static void arm_joint_forward(struct arm *arm, int joint)
{
    struct servo_joint *s = &arm->joints[joint];
    if(s->duty_cycle_percent < servo_limits[joint].max) {
        s->duty_cycle_percent = s->duty_cycle_percent + DUTY_CYCLE_STEP;
        arm_joint_apply_duty_cycle(arm, joint);
    }
}

static void arm_joint_backward(struct arm *arm, int joint)
{
    struct servo_joint *s = &arm->joints[joint];
    if(s->duty_cycle_percent > servo_limits[joint].min) {
        s->duty_cycle_percent = s->duty_cycle_percent - DUTY_CYCLE_STEP;
        arm_joint_apply_duty_cycle(arm, joint);
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
        if(joint == ELEMS(servo_limits) - 1 || joint == ELEMS(servo_limits) - 2) {
            int other = joint == ELEMS(servo_limits) - 1 ?
                        ELEMS(servo_limits) - 2 :
                        ELEMS(servo_limits) - 1;
            switch(maybe_invert(arm, joint, dir)) {
                case JOINT_STILL:
                    break;
                case JOINT_BACKWARD:
                    arm_joint_backward(arm, joint);
                    arm_joint_forward(arm, other);
                    break;
                case JOINT_FORWARD:
                    arm_joint_forward(arm, joint);
                    arm_joint_backward(arm, other);
                    break;
            }
            fprintf(stderr, "%d %"PRIu64"\n", other, arm->joints[other].duty_cycle_percent);
        } else {
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
        }
        arm_sleep();
        fprintf(stderr, "%d %"PRIu64"\n", joint, arm->joints[joint].duty_cycle_percent);
    }

    return 0;
}

struct arm *arm_init(void)
{
    static_assert(ELEMS(servo_limits) == ELEMS(servo_invert_directions) &&
                  ELEMS(servo_limits) == ELEMS(servo_default_duty_cycles),
                  "");

    int ret, npwm;
    struct arm *arm = malloc(sizeof(struct arm));
    if((ret = hwpwm_chip_npwm(pwmchip, &npwm)) < 0)
        fprintf(stderr, "hwpwm_chip_npwm(): %s\n", strerror(hwpwm_error(ret)));
    printf("npwm=%d\n", npwm);
    for(int i = 0; i < npwm; i++)
        if((ret = hwpwm_chip_unexport(pwmchip, i)) < 0)
            fprintf(stderr, "%d hwpwm_chip_unexport(): %s\n", i, strerror(hwpwm_error(ret)));
    for(int i = 0; i < NJOINTS; i++) {
        struct servo_joint *s = &arm->joints[i];
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
                                                       s->duty_cycle_percent)) < 0)
            fprintf(stderr, "hwpwm_channel_set_duty_cycle_percent(): %s\n", strerror(hwpwm_error(ret)));
        if((ret = hwpwm_channel_set_enable(pwmchip, s->channel, true)) < 0)
            fprintf(stderr, "hwpwm_channel_set_enable(): %s\n", strerror(hwpwm_error(ret)));

        arm_joint_apply_duty_cycle(arm, i);
    }
    arm->joint_idx = 0;
    arm->dir = JOINT_STILL;

    mtx_init(&arm->lock, mtx_plain);
    thrd_create(&arm->tid, arm_thread, arm);

    return arm;
}
