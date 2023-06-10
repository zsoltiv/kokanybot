#include <gpiod.h>

#include <stdio.h>

#include "gpio.h"

struct gpiod_chip           *chip;

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
