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
extern struct gpiod_line *left_en,
                  *left_positive,
                  *left_negative;
extern struct gpiod_line *right_en,
                  *right_positive,
                  *right_negative;

void gpio_init(void);
void gpio_cleanup(void);

void motor_stop(void);
void motor_forward(bool pressed);
void motor_backward(bool pressed);
void motor_left(bool pressed);
void motor_right(bool pressed);

#ifdef __cplusplus
}
#endif

#endif /* GPIO_H */
