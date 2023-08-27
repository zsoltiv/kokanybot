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

#include "gpio.h"
#include "offsets.h"
#include "motor.h"

static struct gpiod_line *m1_positive,
                         *m1_negative,
                         *m2_positive,
                         *m2_negative;

static struct gpiod_line *motorpin_init(unsigned int offset)
{
    struct gpiod_line *line = gpiod_chip_get_line(chip, offset);
    if(!line)
        perror("gpiod_chip_get_line()");

    if(gpiod_line_request_output(line,
                                 GPIO_CONSUMER,
                                 GPIO_LOW)) {
        perror("gpiod_line_request_output()");
    }

    gpiod_line_set_value(line, GPIO_LOW);

    return line;
}

void motor_init(void)
{
    m1_positive  = motorpin_init(GPIO17);
    m1_negative  = motorpin_init(GPIO27);
    m2_positive  = motorpin_init(GPIO22);
    m2_negative  = motorpin_init(GPIO23);
}

void motor_cleanup(void)
{
    if(m1_positive) gpiod_line_release(m1_positive);
    if(m1_negative) gpiod_line_release(m1_negative);
    if(m2_positive) gpiod_line_release(m2_positive);
    if(m2_negative) gpiod_line_release(m2_negative);
    if(chip) gpiod_chip_close(chip);
}

void motor_forward(bool pressed)
{
    gpiod_line_set_value(m1_negative,  GPIO_LOW);
    gpiod_line_set_value(m2_negative, GPIO_LOW);
    if(pressed) {
        gpiod_line_set_value(m1_positive,  GPIO_HIGH);
        gpiod_line_set_value(m2_positive, GPIO_HIGH);
    } else {
        gpiod_line_set_value(m1_positive,  GPIO_LOW);
        gpiod_line_set_value(m2_positive, GPIO_LOW);
    }
}

void motor_backward(bool pressed)
{
    gpiod_line_set_value(m1_positive,  GPIO_LOW);
    gpiod_line_set_value(m2_positive, GPIO_LOW);
    if(pressed) {
        gpiod_line_set_value(m1_negative,  GPIO_HIGH);
        gpiod_line_set_value(m2_negative, GPIO_HIGH);
    } else {
        gpiod_line_set_value(m1_negative,  GPIO_LOW);
        gpiod_line_set_value(m2_negative, GPIO_LOW);
    }
}

void motor_left(bool pressed)
{
    gpiod_line_set_value(m1_positive,  GPIO_LOW);
    gpiod_line_set_value(m2_positive, GPIO_LOW);
    if(pressed) {
        gpiod_line_set_value(m1_positive,  GPIO_HIGH);
        gpiod_line_set_value(m2_negative, GPIO_HIGH);
    } else {
        gpiod_line_set_value(m1_positive,  GPIO_LOW);
        gpiod_line_set_value(m2_negative, GPIO_LOW);
    }
}

void motor_right(bool pressed)
{
    gpiod_line_set_value(m1_positive,  GPIO_LOW);
    gpiod_line_set_value(m2_negative, GPIO_LOW);
    if(pressed) {
        gpiod_line_set_value(m1_negative,  GPIO_HIGH);
        gpiod_line_set_value(m2_positive, GPIO_HIGH);
    } else {
        gpiod_line_set_value(m1_negative,  GPIO_LOW);
        gpiod_line_set_value(m2_positive, GPIO_LOW);
    }
}

void motor_stop(bool pressed)
{
    gpiod_line_set_value(m1_negative, GPIO_LOW);
    gpiod_line_set_value(m1_positive, GPIO_LOW);
    gpiod_line_set_value(m2_negative, GPIO_LOW);
    gpiod_line_set_value(m2_positive, GPIO_LOW);
}
