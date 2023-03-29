#include <threads.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "pca9685.h"
#include "servo.h"

struct servo_thread {
    struct servo *servos[ARM_SERVO_COUNT];
    thrd_t thread;
    mtx_t lock;
    int change_pin;
    enum servo_direction change_direction;
};

struct servo {
    int pin;
    int degrees;
    int min_degrees;
    int max_degrees;
    enum servo_direction direction;
};

struct servo_thread *sth;

static inline int servo_calculate_middle_position(struct servo *s)
{
    return s->min_degrees + (s->max_degrees - s->min_degrees) / 2;
}

static struct servo *servo_init(int pin, int min_degrees, int max_degrees)
{
    struct servo *s = malloc(sizeof(struct servo));
    s->pin = pin;
    s->min_degrees = min_degrees;
    s->max_degrees = max_degrees;
    s->degrees = servo_calculate_middle_position(s);
    pca9685_pin_set(s->pin, s->degrees);

    return s;
}

static struct servo *servo_thread_find_change_servo(const struct servo_thread *st)
{
    for(int i = 0; i < ARM_SERVO_COUNT; i++)
        if(st->servos[i]->pin == st->change_pin)
            return st->servos[i];
    return NULL;
}

static void servo_step(struct servo *s)
{
    switch(s->direction) {
        case SERVO_DIRECTION_NONE:
            break;
        case SERVO_DIRECTION_BACKWARD:
            if(s->degrees > s->min_degrees)
                s->degrees--;
            break;
        case SERVO_DIRECTION_FORWARD:
            if(s->degrees < s->max_degrees)
                s->degrees++;
            break;
    }

    pca9685_pin_set(s->pin, s->degrees);
    if(s->pin == 10)
        printf("%d degrees\n", s->degrees);
}

static int servo_thread(void *arg)
{
    struct servo_thread *st = (struct servo_thread *) arg;

    while(1) {
        mtx_lock(&st->lock);

        if(st->change_pin > -1) {
            struct servo *s = servo_thread_find_change_servo(st);
            s->direction = st->change_direction;
            st->change_pin = -1;
        }

        mtx_unlock(&st->lock);

        for(int i = 0; i < ARM_SERVO_COUNT; i++)
            servo_step(st->servos[i]);
        nanosleep(&(struct timespec) {.tv_nsec = 1000000}, NULL);
    }
}

struct servo_thread *servo_thread_init(int servo_pins[static ARM_SERVO_COUNT])
{
    struct servo_thread *st = malloc(sizeof(struct servo_thread));
    mtx_init(&st->lock, mtx_plain);
    st->change_direction = SERVO_DIRECTION_NONE;
    st->change_pin = -1;
    /* FIXME manually pass min and max degrees to each servo_init call */
    st->servos[0] = servo_init(servo_pins[0], 70, 110);
    st->servos[1] = servo_init(servo_pins[1], 60, 160);
    st->servos[2] = servo_init(servo_pins[2], 100, 160);
    st->servos[3] = servo_init(servo_pins[3], 90, 180);
    st->servos[4] = servo_init(servo_pins[4], 100, 150);
    st->servos[5] = servo_init(servo_pins[5], 90, 160);
    st->servos[6] = servo_init(servo_pins[6], 105, 160);
    thrd_create(&st->thread, servo_thread, (void *) st);

    return st;
}

void servo_thread_change(struct servo_thread *st, int pin, enum servo_direction dir)
{
    mtx_lock(&st->lock);

    st->change_pin = pin;
    st->change_direction = dir;

    mtx_unlock(&st->lock);
}

void servo_thread_default(struct servo_thread *st, int pin)
{
    mtx_lock(&st->lock);
    st->change_pin = pin;
    struct servo *s = servo_thread_find_change_servo(st);
    if(!s)
        goto unlock;
    s->degrees = servo_calculate_middle_position(s);
    s->direction = SERVO_DIRECTION_NONE;
    st->change_pin = -1;
unlock:
    mtx_unlock(&st->lock);
}
