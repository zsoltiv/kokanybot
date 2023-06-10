#ifndef GPIO_H
#define GPIO_H

#ifdef __cplusplus
extern "C" {
#endif

#include <gpiod.h>

#include <stdbool.h>

#include "pwm.h"

#define GPIO_CHIP_PATH "/dev/gpiochip0"

enum {
    GPIO_LOW = 0,
    GPIO_HIGH,
};

extern struct gpiod_chip *chip;

void gpio_init(void);
void gpio_cleanup(void);

void motor_forward(bool pressed);
void motor_backward(bool pressed);
void motor_left(bool pressed);
void motor_right(bool pressed);
void motor_stop(bool pressed);

void led_red(void);
void led_green(void);
void led_blue(void);

#ifdef __cplusplus
}
#endif

#endif /* GPIO_H */
