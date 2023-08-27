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
#include <stdlib.h>

#include <gpiod.h>

#include "gpio.h"
#include "stepper.h"

/*
 * Code for working with 28-BYJ48 stepper motors
*/

/* BLUE - PINK - YELLOW - ORANGE */
const int steps[][NPOLES] = {
    { 1, 1, 0, 0 },
    { 0, 1, 1, 0 },
    { 0, 0, 1, 1 },
    { 1, 0, 0, 1 },
};

struct stepper {
    struct gpiod_line *poles[NPOLES];
    int step_idx;
};

static void stepper_set_step(struct stepper *restrict stepper, int i)
{
    for(int j = 0; j < NPOLES; j++)
        if(stepper->poles[j])
            gpiod_line_set_value(stepper->poles[j], steps[i][j]);
    for(int j = 0; j < NPOLES; j++)
        if(!stepper->poles[j])
            gpiod_line_set_value(stepper->poles[j], steps[i][j]);
    stepper->step_idx = i;
}

struct stepper *stepper_init(struct gpiod_chip *chip,
                             const unsigned int pins[static NPOLES])
{
    struct stepper *stepper = malloc(sizeof(struct stepper));

    for(int i = 0; i < NPOLES; i++) {
        stepper->poles[i] = gpiod_chip_get_line(chip, pins[i]);
        gpiod_line_request_output(stepper->poles[i], GPIO_CONSUMER, GPIO_LOW);
    }
    stepper_set_step(stepper, 0);

    return stepper;
}

void stepper_forward(struct stepper *stepper)
{
    int i = stepper->step_idx + 1;
    if(i == NPOLES)
        i = 0;
    stepper_set_step(stepper, i);
}

void stepper_backward(struct stepper *stepper)
{
    int i = stepper->step_idx - 1;
    if(i < 0)
        i = NPOLES - 1;
    stepper_set_step(stepper, i);
}
