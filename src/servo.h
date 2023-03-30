#ifndef SERVO_H
#define SERVO_H

#include <stdbool.h>

#define ARM_SERVO_COUNT 5

enum servo_direction {
    SERVO_DIRECTION_NONE = 0,
    SERVO_DIRECTION_FORWARD,
    SERVO_DIRECTION_BACKWARD,
};

struct servo_thread;

extern struct servo_thread *sth;

struct servo_thread *servo_thread_init(int servo_pins[static ARM_SERVO_COUNT]);
void servo_thread_change(struct servo_thread *st, int pin, enum servo_direction dir);
void servo_thread_default(struct servo_thread *st, int pin);

#endif /* SERVO_H */
