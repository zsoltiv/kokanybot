#include <threads.h>
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
    enum servo_direction direction;
};

struct servo_thread *sth;

static struct servo *servo_init(int pin, int degrees)
{
    struct servo *s = malloc(sizeof(struct servo));
    s->pin = pin;
    s->degrees = degrees;
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
            if(s->degrees > 1)
                s->degrees--;
            break;
        case SERVO_DIRECTION_FORWARD:
            if(s->degrees < 179)
                s->degrees++;
            break;
    }

    pca9685_pin_set(s->pin, s->degrees);
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
    for(int i = 0; i < ARM_SERVO_COUNT; i++)
        st->servos[i] = servo_init(servo_pins[i], 90);
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
    s->degrees = 90;
    s->direction = SERVO_DIRECTION_NONE;
    st->change_pin = -1;
unlock:
    mtx_unlock(&st->lock);
}
