#include <gpiod.h>

#include <stdio.h>
#include <errno.h>
#include <string.h>

#include "offsets.h"
#include "pwm.h"
#include "gpio.h"

/* PWM constants used for RGB LED */
#define PWM_FREQUENCY 50
#define PWM_DUTY_MAX 256

struct gpiod_chip           *chip;
struct gpiod_line        *m1_en,
                   *m1_positive,
                   *m1_negative,
                         *m2_en,
                   *m2_positive,
                   *m2_negative,
                         *m3_en,
                   *m3_positive,
                   *m3_negative,
                         *m4_en,
                   *m4_positive,
                   *m4_negative;
struct pwm *r,
           *g,
           *b;

// debugra
static int my_line_set_value(struct gpiod_line *line, int value)
{
    int ret = gpiod_line_set_value(line, value);
    if(ret < 0)
        perror("gpiod_line_set_value error");
    return ret;
}

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
        //gpio_err();
    }

    m1_en        = motorpin_init( GPIO5);
    m1_positive  = motorpin_init(GPIO24);
    m1_negative  = motorpin_init(GPIO27);
    m2_en        = motorpin_init(GPIO17);
    m2_positive  = motorpin_init( GPIO6);
    m2_negative  = motorpin_init(GPIO22);
    m3_en        = motorpin_init(GPIO12);
    m3_positive  = motorpin_init(GPIO23);
    m3_negative  = motorpin_init(GPIO16);
    m4_en        = motorpin_init(GPIO25);
    m4_positive  = motorpin_init(GPIO13);
    m4_negative  = motorpin_init(GPIO18);

    r = pwm_init(chip, SPI_SCLK, PWM_FREQUENCY, PWM_DUTY_MAX, 0);
    g = pwm_init(chip, SPI_MISO, PWM_FREQUENCY, PWM_DUTY_MAX, 0);
    b = pwm_init(chip, SPI_MOSI, PWM_FREQUENCY, PWM_DUTY_MAX, 0);

    gpiod_line_set_value(m1_en, GPIO_HIGH);
    gpiod_line_set_value(m2_en, GPIO_HIGH);
    gpiod_line_set_value(m3_en, GPIO_HIGH);
    gpiod_line_set_value(m4_en, GPIO_HIGH);
}

void gpio_cleanup(void)
{
    gpiod_line_set_value(m1_en,        GPIO_LOW);
    gpiod_line_set_value(m2_en,       GPIO_LOW);

    if(m1_en) gpiod_line_release(m1_en);
    if(m1_positive) gpiod_line_release(m1_positive);
    if(m1_negative) gpiod_line_release(m1_negative);
    if(m2_en) gpiod_line_release(m2_en);
    if(m2_positive) gpiod_line_release(m2_positive);
    if(m2_negative) gpiod_line_release(m2_negative);
    if(m3_en) gpiod_line_release(m3_en);
    if(m3_positive) gpiod_line_release(m3_positive);
    if(m3_negative) gpiod_line_release(m3_negative);
    if(m4_en) gpiod_line_release(m4_en);
    if(m4_positive) gpiod_line_release(m4_positive);
    if(m4_negative) gpiod_line_release(m4_negative);
    if(chip) gpiod_chip_close(chip);
}

void motor_forward(bool pressed)
{
    gpiod_line_set_value(m1_negative,  GPIO_LOW);
    gpiod_line_set_value(m4_negative,  GPIO_LOW);
    gpiod_line_set_value(m2_negative, GPIO_LOW);
    gpiod_line_set_value(m3_positive, GPIO_LOW);
    if(pressed) {
        gpiod_line_set_value(m1_positive,  GPIO_HIGH);
        gpiod_line_set_value(m4_positive,  GPIO_HIGH);
        gpiod_line_set_value(m2_positive, GPIO_HIGH);
        gpiod_line_set_value(m3_negative, GPIO_HIGH);
    } else {
        gpiod_line_set_value(m1_positive,  GPIO_LOW);
        gpiod_line_set_value(m4_positive,  GPIO_LOW);
        gpiod_line_set_value(m2_positive, GPIO_LOW);
        gpiod_line_set_value(m3_negative, GPIO_LOW);
    }
}

void motor_backward(bool pressed)
{
    gpiod_line_set_value(m1_positive,  GPIO_LOW);
    gpiod_line_set_value(m4_positive,  GPIO_LOW);
    gpiod_line_set_value(m2_positive, GPIO_LOW);
    gpiod_line_set_value(m3_negative, GPIO_LOW);
    if(pressed) {
        gpiod_line_set_value(m1_negative,  GPIO_HIGH);
        gpiod_line_set_value(m4_negative,  GPIO_HIGH);
        gpiod_line_set_value(m2_negative, GPIO_HIGH);
        gpiod_line_set_value(m3_positive, GPIO_HIGH);
    } else {
        gpiod_line_set_value(m1_negative,  GPIO_LOW);
        gpiod_line_set_value(m4_negative,  GPIO_LOW);
        gpiod_line_set_value(m2_negative, GPIO_LOW);
        gpiod_line_set_value(m3_positive, GPIO_LOW);
    }
}

void motor_left(bool pressed)
{
    gpiod_line_set_value(m1_positive,  GPIO_LOW);
    gpiod_line_set_value(m4_negative,  GPIO_LOW);
    gpiod_line_set_value(m2_positive, GPIO_LOW);
    gpiod_line_set_value(m3_positive, GPIO_LOW);
    if(pressed) {
        gpiod_line_set_value(m1_positive,  GPIO_HIGH);
        gpiod_line_set_value(m4_negative,  GPIO_HIGH);
        gpiod_line_set_value(m2_negative, GPIO_HIGH);
        gpiod_line_set_value(m3_negative, GPIO_HIGH);
    } else {
        gpiod_line_set_value(m1_positive,  GPIO_LOW);
        gpiod_line_set_value(m4_negative,  GPIO_LOW);
        gpiod_line_set_value(m2_negative, GPIO_LOW);
        gpiod_line_set_value(m3_negative, GPIO_LOW);
    }
}

void motor_right(bool pressed)
{
    gpiod_line_set_value(m1_positive,  GPIO_LOW);
    gpiod_line_set_value(m4_negative,  GPIO_LOW);
    gpiod_line_set_value(m2_negative, GPIO_LOW);
    gpiod_line_set_value(m3_negative, GPIO_LOW);
    if(pressed) {
        gpiod_line_set_value(m1_negative,  GPIO_HIGH);
        gpiod_line_set_value(m4_positive,  GPIO_HIGH);
        gpiod_line_set_value(m2_positive, GPIO_HIGH);
        gpiod_line_set_value(m3_positive, GPIO_HIGH);
    } else {
        gpiod_line_set_value(m1_negative,  GPIO_LOW);
        gpiod_line_set_value(m4_positive,  GPIO_LOW);
        gpiod_line_set_value(m2_positive, GPIO_LOW);
        gpiod_line_set_value(m3_positive, GPIO_LOW);
    }
}

void motor_stop(bool pressed)
{
    gpiod_line_set_value(m1_negative, GPIO_LOW);
    gpiod_line_set_value(m1_positive, GPIO_LOW);
    gpiod_line_set_value(m2_negative, GPIO_LOW);
    gpiod_line_set_value(m2_positive, GPIO_LOW);
    gpiod_line_set_value(m3_negative, GPIO_LOW);
    gpiod_line_set_value(m3_positive, GPIO_LOW);
    gpiod_line_set_value(m4_negative, GPIO_LOW);
    gpiod_line_set_value(m4_positive, GPIO_LOW);
}
