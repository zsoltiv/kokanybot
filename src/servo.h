#ifndef SERVO_H
#define SERVO_H

#include <stdbool.h>

#define ARM_SERVO_COUNT 7

enum servo_direction {
    SERVO_DIRECTION_NONE = 0,
    SERVO_DIRECTION_FORWARD,
    SERVO_DIRECTION_BACKWARD,
};

struct servo_thread;

struct servo_thread *servo_thread_init(int servo_pins[static ARM_SERVO_COUNT]);

#endif /* SERVO_H */
