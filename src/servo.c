#include <threads.h>
#include <stdlib.h>
#include <time.h>

#include "pca9685.h"
#include "servo.h"

struct servo {
    int pin;
    int degrees;
    enum servo_direction direction;
};

struct servo *servo_init(int pin, int degrees)
{
    struct servo *s = malloc(sizeof(struct servo));
    s->pin = pin;
    s->degrees = degrees;
    pca9685_pin_set(s->pin, s->degrees);

    return s;
}

void servo_move(struct servo *s, enum servo_direction dir)
{
    switch(dir) {
        case SERVO_DIRECTION_FORWARD:
            if(s->degrees < 179)
                s->degrees++;
            break;
        case SERVO_DIRECTION_BACKWARD:
            if(s->degrees > 0)
                s->degrees--;
            break;
        default:
            break;
    }
    pca9685_pin_set(s->pin, s->degrees);
}
