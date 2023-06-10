#ifndef STEPPER_H
#define STEPPER_H

#ifdef __cplusplus
extern "C" {
#endif

#include <gpiod.h>

#define NPOLES 4

struct stepper;

struct stepper *stepper_init(struct gpiod_chip *chip,
                             unsigned int pins[static NPOLES]);
void stepper_forward(struct stepper *stepper);
void stepper_backward(struct stepper *stepper);

#ifdef __cplusplus
}
#endif

#endif /* STEPPER_H */
