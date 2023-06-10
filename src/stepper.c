#include <stdlib.h>

#include <gpiod.h>

#include "gpio.h"
#include "stepper.h"

/*
 * Code for working with 28-BYJ48 stepper motors
*/

/* BLUE - PINK - YELLOW - ORANGE */
static const int steps[][NPOLES] = {
    { 1, 1, 0, 0 },
    { 0, 1, 1, 0 },
    { 0, 0, 1, 1 },
    { 1, 0, 0, 1 },
};

struct stepper {
    struct gpiod_line *poles[NPOLES];
    int step_idx;
};

static void stepper_set_step(struct stepper *restrict stepper, int i)
{
    for(int j = 0; j < NPOLES; j++)
        gpiod_line_set_value(stepper->poles[j], steps[i][j]);
    stepper->step_idx = i;
}

struct stepper *stepper_init(struct gpiod_chip *chip,
                             unsigned int pins[static NPOLES])
{
    struct stepper *stepper = malloc(sizeof(struct stepper));

    for(int i = 0; i < NPOLES; i++) {
        stepper->poles[i] = gpiod_chip_get_line(chip, pins[i]);
        gpiod_line_request_output(stepper->poles[i], GPIO_CONSUMER, GPIO_LOW);
    }
    stepper_set_step(stepper, 0);

    return stepper;
}

void stepper_forward(struct stepper *stepper)
{
    int i = stepper->step_idx + 1;
    if(i == NPOLES)
        i = 0;
    stepper_set_step(stepper, i);
}

void stepper_backward(struct stepper *stepper)
{
    int i = stepper->step_idx - 1;
    if(i < 0)
        i = NPOLES - 1;
    stepper_set_step(stepper, i);
}
