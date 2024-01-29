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

void gpio_init(void)
{
    chip = gpiod_chip_open(GPIO_CHIP_PATH);
    if(!chip) {
        perror("gpiod_chip_open()");
    }
}

struct gpiod_line_request *gpio_init_line(size_t pin_count,
                                          const unsigned pins[static pin_count],
                                          enum gpiod_line_direction dir)
{
    struct gpiod_request_config *req_cfg = gpiod_request_config_new();
    gpiod_request_config_set_consumer(req_cfg, GPIO_CONSUMER);
    struct gpiod_line_settings *line_settings = gpiod_line_settings_new();
    gpiod_line_settings_set_direction(line_settings, dir);
    if(dir == GPIOD_LINE_DIRECTION_OUTPUT)
        gpiod_line_settings_set_output_value(line_settings, 0);
    struct gpiod_line_config *line_cfg = gpiod_line_config_new();
    gpiod_line_config_add_line_settings(line_cfg, pins, pin_count, line_settings);
    struct gpiod_line_request *line = gpiod_chip_request_lines(chip, req_cfg, line_cfg);
    gpiod_line_settings_free(line_settings);
    gpiod_line_config_free(line_cfg);
    gpiod_request_config_free(req_cfg);
    return line;
}

struct gpiod_line_request *gpio_init_input_events(const unsigned pin, enum gpiod_line_edge events)
{
    struct gpiod_request_config *req_cfg = gpiod_request_config_new();
    gpiod_request_config_set_consumer(req_cfg, GPIO_CONSUMER);
    struct gpiod_line_settings *line_settings = gpiod_line_settings_new();
    gpiod_line_settings_set_direction(line_settings, GPIOD_LINE_DIRECTION_INPUT);
    gpiod_line_settings_set_edge_detection(line_settings, events);
    struct gpiod_line_config *line_cfg = gpiod_line_config_new();
    gpiod_line_config_add_line_settings(line_cfg, &pin, 1, line_settings);
    struct gpiod_line_request *line = gpiod_chip_request_lines(chip, req_cfg, line_cfg);
    gpiod_line_settings_free(line_settings);
    gpiod_line_config_free(line_cfg);
    gpiod_request_config_free(req_cfg);
    return line;
}
