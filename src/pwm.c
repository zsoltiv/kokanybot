#include <threads.h>
#include <stdio.h>
#include <stdbool.h>
#include <gpiod.h>
#include <time.h>

#include "pwm.h"

#define ONESEC 1000000000L

struct pwm {
    int frequency; // given in Hz
    int duty_max; // maximum value for `duty_cycle`
    int duty_cycle; 
    bool duty_change;
    thrd_t thread;
    mtx_t lock;
    struct gpiod_line *gpio;
};

static int pwm_thread(void *arg)
{
    struct pwm *pwm = (struct pwm *) arg;
    long duty_unit = ONESEC / pwm->frequency / pwm->duty_max;
    struct timespec on_time = {0}, off_time = {0};
    while(1) {
        mtx_lock(&pwm->lock);
        if(pwm->duty_change) {
            on_time.tv_nsec = duty_unit * pwm->duty_cycle;
            off_time.tv_nsec = duty_unit * (pwm->duty_max - pwm->duty_cycle);
            pwm->duty_change = false;
        }
        mtx_unlock(&pwm->lock);

        // NOTE can only change PWM every second
        for(int period = 0; period < pwm->frequency; period++) {
            gpiod_line_set_value(pwm->gpio, 1);
            nanosleep(&on_time, NULL);
            gpiod_line_set_value(pwm->gpio, 0);
            nanosleep(&off_time, NULL);
        }
    }

    return 0;
}

struct pwm *pwm_init(struct gpiod_chip *chip,
                     unsigned int gpio,
                     int frequency,
                     int duty_max,
                     int duty_cycle)
{
    struct pwm *pwm = malloc(sizeof(struct pwm));
    pwm->duty_cycle = duty_cycle;
    pwm->duty_max = duty_max;
    pwm->duty_change = true;
    pwm->frequency = frequency;
    pwm->gpio = gpiod_chip_get_line(chip, gpio);
    printf("freq %d\tduty_max %d\tduty_cycle %d\n",
           pwm->frequency,
           pwm->duty_max,
           pwm->duty_cycle);
    if(!pwm->gpio)
        perror("gpiod_chip_get_line()");
    if(gpiod_line_request_output(pwm->gpio,
                                 GPIO_CONSUMER,
                                 0) < 0)
        perror("gpiod_line_request_output()");
    mtx_init(&pwm->lock, mtx_plain);
    thrd_create(&pwm->thread, pwm_thread, (void *) pwm);
    return pwm;
}

void pwm_set_duty_cycle(struct pwm *pwm, int duty_cycle)
{
    mtx_lock(&pwm->lock);
    pwm->duty_change = true;
    pwm->duty_cycle = duty_cycle;
    mtx_unlock(&pwm->lock);
}
