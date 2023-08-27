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
#include <gpiod.h>

#include <stdio.h>

#include "gpio.h"

struct gpiod_chip *chip;

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

void gpio_init(void)
{
    chip = gpiod_chip_open(GPIO_CHIP_PATH);
    if(!chip) {
        perror("gpiod_chip_open()");
    }
}
