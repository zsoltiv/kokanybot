#ifndef GPIO_H
#define GPIO_H

#include <gpiod.h>

#define GPIO_CHIP_PATH "/dev/gpiochip0"
#define GPIO_CONSUMER "kokanybot"

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
void motor_forward(void);
void motor_backward(void);
void motor_left(void);
void motor_right(void);

#endif /* GPIO_H */
