#ifndef PWM_H
#define PWM_H

#ifdef __cplusplus
extern "C" {
#endif

/* Software PWM implementation for controlling an RGB LED */

#include <gpiod.h>

#define GPIO_CONSUMER "kokanybot"

struct pwm;

struct pwm *pwm_init(struct gpiod_chip *chip,
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
