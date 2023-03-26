#ifndef SERVO_H
#define SERVO_H

#include <stdbool.h>

enum servo_direction {
    SERVO_DIRECTION_NONE = 0,
    SERVO_DIRECTION_FORWARD,
    SERVO_DIRECTION_BACKWARD,
};

struct servo;

struct servo *servo_init(int pin, int degrees);
void servo_move(struct servo *s, enum servo_direction dir);

#endif /* SERVO_H */
