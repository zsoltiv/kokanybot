#include <gpiod.h>

#include <stdio.h>

#include "gpio.h"
#include "offsets.h"

struct gpiod_chip           *chip;
struct gpiod_line        *left_en,
                   *left_positive,
                   *left_negative;
struct gpiod_line       *right_en,
                  *right_positive,
                  *right_negative;

static void gpio_err(void)
{
    gpio_cleanup();
}

static struct gpiod_line *motorpin_init(unsigned int offset)
{
    struct gpiod_line *line = gpiod_chip_get_line(chip, offset);
    if(!line) {
        perror("gpiod_chip_get_line()");
        exit(1);
    }

    if(gpiod_line_request_output(line,
                                 GPIO_CONSUMER,
                                 GPIO_LOW)) {
        perror("gpiod_line_request_output()");
        gpio_err();
    }

    return line;
}

void gpio_init(void)
{
    chip = gpiod_chip_open(GPIO_CHIP_PATH);
    if(!chip) {
        perror("gpiod_chip_open()");
        gpio_err();
    }

    left_en        = motorpin_init( GPIO5);
    left_positive  = motorpin_init(GPIO24);
    left_negative  = motorpin_init(GPIO27);
    right_en       = motorpin_init(GPIO17);
    right_positive = motorpin_init( GPIO6);
    right_negative = motorpin_init(GPIO22);

    gpiod_line_set_value(left_en,        GPIO_HIGH);
    gpiod_line_set_value(right_en,       GPIO_HIGH);
}

void gpio_cleanup(void)
{
    gpiod_line_set_value(left_en,        GPIO_LOW);
    gpiod_line_set_value(right_en,       GPIO_LOW);

    if(left_en) gpiod_line_release(left_en);
    if(left_positive) gpiod_line_release(left_positive);
    if(left_negative) gpiod_line_release(left_negative);
    if(right_en) gpiod_line_release(right_en);
    if(right_positive) gpiod_line_release(right_positive);
    if(right_negative) gpiod_line_release(right_negative);
    if(chip) gpiod_chip_close(chip);
}

void motor_forward(bool pressed)
{
    gpiod_line_set_value(left_negative,  GPIO_LOW);
    gpiod_line_set_value(right_negative, GPIO_LOW);
    if(pressed) {
        gpiod_line_set_value(left_positive,  GPIO_HIGH);
        gpiod_line_set_value(right_positive, GPIO_HIGH);
    } else {
        gpiod_line_set_value(left_positive,  GPIO_LOW);
        gpiod_line_set_value(right_positive, GPIO_LOW);
    }
}

void motor_backward(bool pressed)
{
    gpiod_line_set_value(left_positive,  GPIO_LOW);
    gpiod_line_set_value(right_positive, GPIO_LOW);
    if(pressed) {
        gpiod_line_set_value(left_negative,  GPIO_HIGH);
        gpiod_line_set_value(right_negative, GPIO_HIGH);
    } else {
        gpiod_line_set_value(left_negative,  GPIO_LOW);
        gpiod_line_set_value(right_negative, GPIO_LOW);
    }
}

void motor_left(bool pressed)
{
    gpiod_line_set_value(left_negative,  GPIO_LOW);
    gpiod_line_set_value(right_positive, GPIO_LOW);
    if(pressed) {
        gpiod_line_set_value(left_positive,  GPIO_HIGH);
        gpiod_line_set_value(right_negative, GPIO_HIGH);
    } else {
        gpiod_line_set_value(left_positive,  GPIO_LOW);
        gpiod_line_set_value(right_negative, GPIO_LOW);
    }
}

void motor_right(bool pressed)
{
    gpiod_line_set_value(left_positive,  GPIO_LOW);
    gpiod_line_set_value(right_negative, GPIO_LOW);
    if(pressed) {
        gpiod_line_set_value(left_negative,  GPIO_HIGH);
        gpiod_line_set_value(right_positive, GPIO_HIGH);
    } else {
        gpiod_line_set_value(left_negative,  GPIO_LOW);
        gpiod_line_set_value(right_positive, GPIO_LOW);
    }
}
