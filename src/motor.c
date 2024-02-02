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

static const unsigned m1_positive = GPIO24,
                      m1_negative = GPIO27,
                      m1_en       = GPIO5,
                      m2_positive = GPIO6,
                      m2_negative = GPIO22,
                      m2_en       = GPIO17;

static const unsigned motor_offsets[] = {
    m1_positive, m1_negative, m1_en, m2_positive, m2_negative, m2_en
};

static struct gpiod_line_request *motor_req;

void motor_init(void)
{
    motor_req = gpio_init_line(chip,
                               sizeof(motor_offsets) / sizeof(motor_offsets[0]),
                               motor_offsets,
                               GPIOD_LINE_DIRECTION_OUTPUT);
    gpiod_line_request_set_value(motor_req, m1_en, 1);
    gpiod_line_request_set_value(motor_req, m2_en, 1);
}

void motor_cleanup(void)
{
    if(motor_req) gpiod_line_request_release(motor_req);
    if(chip) gpiod_chip_close(chip);
}

void motor_forward(bool pressed)
{
    gpiod_line_request_set_value(motor_req, m1_negative,  GPIO_LOW);
    gpiod_line_request_set_value(motor_req, m2_negative, GPIO_LOW);
    if(pressed) {
        gpiod_line_request_set_value(motor_req, m1_positive,  GPIO_HIGH);
        gpiod_line_request_set_value(motor_req, m2_positive, GPIO_HIGH);
    } else {
        gpiod_line_request_set_value(motor_req, m1_positive,  GPIO_LOW);
        gpiod_line_request_set_value(motor_req, m2_positive, GPIO_LOW);
    }
}

void motor_backward(bool pressed)
{
    gpiod_line_request_set_value(motor_req, m1_positive,  GPIO_LOW);
    gpiod_line_request_set_value(motor_req, m2_positive, GPIO_LOW);
    if(pressed) {
        gpiod_line_request_set_value(motor_req, m1_negative,  GPIO_HIGH);
        gpiod_line_request_set_value(motor_req, m2_negative, GPIO_HIGH);
    } else {
        gpiod_line_request_set_value(motor_req, m1_negative,  GPIO_LOW);
        gpiod_line_request_set_value(motor_req, m2_negative, GPIO_LOW);
    }
}

void motor_left(bool pressed)
{
    gpiod_line_request_set_value(motor_req, m1_positive,  GPIO_LOW);
    gpiod_line_request_set_value(motor_req, m2_positive, GPIO_LOW);
    if(pressed) {
        gpiod_line_request_set_value(motor_req, m1_positive,  GPIO_HIGH);
        gpiod_line_request_set_value(motor_req, m2_negative, GPIO_HIGH);
    } else {
        gpiod_line_request_set_value(motor_req, m1_positive,  GPIO_LOW);
        gpiod_line_request_set_value(motor_req, m2_negative, GPIO_LOW);
    }
}

void motor_right(bool pressed)
{
    gpiod_line_request_set_value(motor_req, m1_positive,  GPIO_LOW);
    gpiod_line_request_set_value(motor_req, m2_negative, GPIO_LOW);
    if(pressed) {
        gpiod_line_request_set_value(motor_req, m1_negative,  GPIO_HIGH);
        gpiod_line_request_set_value(motor_req, m2_positive, GPIO_HIGH);
    } else {
        gpiod_line_request_set_value(motor_req, m1_negative,  GPIO_LOW);
        gpiod_line_request_set_value(motor_req, m2_positive, GPIO_LOW);
    }
}

void motor_stop(bool pressed)
{
    gpiod_line_request_set_value(motor_req, m1_negative, GPIO_LOW);
    gpiod_line_request_set_value(motor_req, m1_positive, GPIO_LOW);
    gpiod_line_request_set_value(motor_req, m2_negative, GPIO_LOW);
    gpiod_line_request_set_value(motor_req, m2_positive, GPIO_LOW);
}
