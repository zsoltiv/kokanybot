#ifndef PWM_H
#define PWM_H

#ifdef __cplusplus
extern "C" {
#endif

/* Software PWM implementation for controlling an RGB LED */

#include <threads.h>
#include <gpiod.h>

struct pwm {
    int frequency; // given in Hz
    int duty_max; // maximum value for `duty_cycle`
    int duty_cycle; 
    thrd_t thread;
    mtx_t lock;
    struct gpiod_line *gpio;
};

void pwm_init(struct pwm *pwm,
              struct gpiod_chip *chip,
              unsigned int gpio,
              int frequency,
              int duty_max,
              int duty_cycle);

/*
 * MUST be used to change the duty cycle
 * handles locking for the pwm object
*/
void pwm_set_duty_cycle(struct pwm *pwm, int duty_cycle);

#ifdef __cplusplus
}
#endif

#endif
