#include <threads.h>
#include <gpiod.h>
#include <time.h>

#include "pwm.h"

#define ONESEC 1000000000L

static int pwm_thread(void *arg)
{
    struct pwm *pwm = (struct pwm *) arg;
    long duty_unit = ONESEC / pwm->frequency / pwm->duty_max;

    while(1) {
        mtx_lock(&pwm->lock);
        struct timespec on_time = {
            .tv_nsec = duty_unit * pwm->duty_cycle,
        };
        struct timespec off_time = {
            .tv_nsec = duty_unit * (pwm->duty_max - pwm->duty_cycle),
        };
        mtx_unlock(&pwm->lock);
        gpiod_line_set_value(pwm->gpio, GPIOD_LINE_ACTIVE_STATE_HIGH);
        nanosleep(&on_time, NULL);
        gpiod_line_set_value(pwm->gpio, GPIOD_LINE_ACTIVE_STATE_LOW);
        nanosleep(&off_time, NULL);
    }

    return 0;
}

void pwm_init(struct pwm *pwm,
              struct gpiod_chip *chip,
              unsigned int gpio,
              int frequency,
              int duty_max,
              int duty_cycle)
{
    pwm->duty_cycle = duty_cycle;
    pwm->duty_max = duty_max;
    pwm->frequency = frequency;
    thrd_create(&pwm->thread, pwm_thread, (void *) pwm);
    mtx_init(&pwm->lock, mtx_plain);
    pwm->gpio = gpiod_chip_get_line(chip, gpio);
}
void pwm_set_duty_cycle(struct pwm *pwm, int duty_cycle)
{
    mtx_lock(&pwm->lock);
    pwm->duty_cycle = duty_cycle;
    mtx_unlock(&pwm->lock);
}
