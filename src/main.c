#include <stdio.h>
#include <stdlib.h>
#include <threads.h>
#include <time.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <linux/input-event-codes.h>

#include "init.h"
#include "pca9685.h"
#include "gpio.h"
#include "input.h"
#include "servo.h"
#include "pwm.h"

mtx_t init_mtx;
int servo_pins[] = {11, 12, 13, 14, 15};

void stop(bool unused)
{
    dispatch_pending_events();
    motor_stop(unused);
    for(int i = 0; i < 16; i++)
        servo_thread_default(sth, i);
}

static int pin = 15;

#define SERVO_SELECT_FUNC(n) static void servo_select_##n(bool unused) \
{\
    pin = servo_pins[sizeof(servo_pins) / sizeof(servo_pins[0]) - (n)];\
}

#define SERVO_SELECT_BIND(n) { .key = KEY_##n, .func = servo_select_##n }

SERVO_SELECT_FUNC(1)
SERVO_SELECT_FUNC(2)
SERVO_SELECT_FUNC(3)
SERVO_SELECT_FUNC(4)
SERVO_SELECT_FUNC(5)

void move_servo_forward(bool pressed)
{
    servo_thread_change(sth, pin, pressed ? SERVO_DIRECTION_FORWARD : SERVO_DIRECTION_NONE);
}
void move_servo_backward(bool pressed)
{
    servo_thread_change(sth, pin, pressed ? SERVO_DIRECTION_BACKWARD : SERVO_DIRECTION_NONE);
}

struct key_bind key_binds[INPUT_KEY_BINDS] = {
    { .key =     KEY_W, .func =        motor_forward },
    { .key =     KEY_A, .func =           motor_left },
    { .key =     KEY_S, .func =       motor_backward },
    { .key =     KEY_D, .func =          motor_right },
    { .key =     KEY_R, .func =                 stop },
    { .key =     KEY_E, .func = move_servo_forward   },
    { .key =     KEY_Q, .func = move_servo_backward  },
    SERVO_SELECT_BIND(1),
    SERVO_SELECT_BIND(2),
    SERVO_SELECT_BIND(3),
    SERVO_SELECT_BIND(4),
    SERVO_SELECT_BIND(5),
};

int main(void)
{
    gpio_init();
    pca9685_init();
    input_init();
    sth = servo_thread_init(servo_pins);
    mtx_init(&init_mtx, mtx_plain);
    printf("Up and running\n");
    mtx_lock(&init_mtx);
    while(1) { // robot loop
        input_receive_input();
    }
    mtx_unlock(&init_mtx);
}
