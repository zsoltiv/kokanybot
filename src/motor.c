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

#include <gpiod.h>
#define _XOPEN_SOURCE 700
#include <stdio.h>

#include "gpio.h"
#include "offsets.h"
#include "motor.h"

#define NMOTORS 4

static const unsigned m1_positive = GPIO24,
                      m1_negative = GPIO27,
                      m1_en       = GPIO5,
                      m2_positive = GPIO6,
                      m2_negative = GPIO22,
                      m2_en       = GPIO17,
                      m3_positive = GPIO16,
                      m3_negative = GPIO23,
                      m3_en       = GPIO12,
                      m4_positive = GPIO13,
                      m4_negative = GPIO18,
                      m4_en       = GPIO25;

static const unsigned motor_offsets[] = {
    m1_positive, m1_negative, m1_en,
    m2_positive, m2_negative, m2_en,
    m3_positive, m3_negative, m3_en,
    m4_positive, m4_negative, m4_en,
};

static struct gpiod_line_request *motor_req;

static const enum gpiod_line_value values_low[NMOTORS]  = { 0, 0, 0, 0 };
static const enum gpiod_line_value values_high[NMOTORS] = { 1, 1, 1, 1 };

void motor_init(void)
{
    motor_req = gpio_init_line(chip,
                               sizeof(motor_offsets) / sizeof(motor_offsets[0]),
                               motor_offsets,
                               GPIOD_LINE_DIRECTION_OUTPUT);
    gpiod_line_request_set_values_subset(motor_req,
                                         NMOTORS,
                                         (const unsigned[]){m1_en,m2_en,m3_en,m4_en},
                                         values_high);
}

void motor_cleanup(void)
{
    if(motor_req) gpiod_line_request_release(motor_req);
    if(chip) gpiod_chip_close(chip);
}

void motor_forward(bool pressed)
{
    gpiod_line_request_set_values_subset(motor_req,
                                         NMOTORS,
                                         (const unsigned[]){m1_positive,m2_positive,m3_negative,m4_negative},
                                         values_low);
    if(pressed) {
    gpiod_line_request_set_values_subset(motor_req,
                                         NMOTORS,
                                         (const unsigned[]){m1_negative,m2_negative,m3_positive,m4_positive},
                                         values_high);
    } else {
    gpiod_line_request_set_values_subset(motor_req,
                                         NMOTORS,
                                         (const unsigned[]){m1_negative,m2_negative,m3_positive,m4_positive},
                                         values_low);
    }
}

void motor_backward(bool pressed)
{
    gpiod_line_request_set_values_subset(motor_req,
                                         NMOTORS,
                                         (const unsigned[]){m1_negative,m2_negative,m3_positive,m4_positive},
                                         values_low);
    if(pressed) {
    gpiod_line_request_set_values_subset(motor_req,
                                         NMOTORS,
                                         (const unsigned[]){m1_positive,m2_positive,m3_negative,m4_negative},
                                         values_high);
    } else {
    gpiod_line_request_set_values_subset(motor_req,
                                         NMOTORS,
                                         (const unsigned[]){m1_positive,m2_positive,m3_negative,m4_negative},
                                         values_low);
    }
}

void motor_left(bool pressed)
{
    gpiod_line_request_set_values_subset(motor_req,
                                         NMOTORS,
                                         (const unsigned[]){m1_negative,m2_positive,m3_negative,m4_positive},
                                         values_low);
    if(pressed) {
    gpiod_line_request_set_values_subset(motor_req,
                                         NMOTORS,
                                         (const unsigned[]){m1_positive,m2_negative,m3_positive,m4_negative},
                                         values_high);
    } else {
    gpiod_line_request_set_values_subset(motor_req,
                                         NMOTORS,
                                         (const unsigned[]){m1_positive,m2_negative,m3_positive,m4_negative},
                                         values_low);
    }
}

void motor_right(bool pressed)
{
    gpiod_line_request_set_values_subset(motor_req,
                                         NMOTORS,
                                         (const unsigned[]){m1_positive,m2_negative,m3_positive,m4_negative},
                                         values_low);
    if(pressed) {
    gpiod_line_request_set_values_subset(motor_req,
                                         NMOTORS,
                                         (const unsigned[]){m1_negative,m2_positive,m3_negative,m4_positive},
                                         values_high);
    } else {
    gpiod_line_request_set_values_subset(motor_req,
                                         NMOTORS,
                                         (const unsigned[]){m1_negative,m2_positive,m3_negative,m4_positive},
                                         values_low);
    }
}
