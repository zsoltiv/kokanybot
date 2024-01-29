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

#ifndef GPIO_H
#define GPIO_H

#ifdef __cplusplus
extern "C" {
#endif

#include <gpiod.h>

#include <stdbool.h>

#define GPIO_CHIP_PATH "/dev/gpiochip0"
#define GPIO_CONSUMER "kokanybot"

enum {
    GPIO_LOW = 0,
    GPIO_HIGH,
};

extern struct gpiod_chip *chip;

void gpio_init(void);
struct gpiod_line_request *gpio_init_line(size_t pin_count,
                                          const unsigned pins[static pin_count],
                                          enum gpiod_line_direction dir);
struct gpiod_line_request *gpio_init_input_events(const unsigned pin, enum gpiod_line_edge events);

#ifdef __cplusplus
}
#endif

#endif /* GPIO_H */
