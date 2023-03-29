#include <stdio.h>
#include <stdlib.h>
#include <threads.h>
#include <time.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <linux/input-event-codes.h>

#include "img.h"
#include "pca9685.h"
#include "gpio.h"
#include "input.h"
#include "servo.h"
#include "pwm.h"

void stop(bool unused)
{
    dispatch_pending_events();
    motor_stop(unused);
    for(int i = 0; i < 16; i++)
        servo_thread_default(sth, i);
}

void move_servo_forward(bool pressed)
{
    servo_thread_change(sth, 10, pressed ? SERVO_DIRECTION_FORWARD : SERVO_DIRECTION_NONE);
}
void move_servo_backward(bool pressed)
{
    servo_thread_change(sth, 10, pressed ? SERVO_DIRECTION_BACKWARD : SERVO_DIRECTION_NONE);
}

struct key_bind key_binds[INPUT_KEY_BINDS] = {
    { .key =     KEY_W, .func =        motor_forward },
    { .key =     KEY_A, .func =           motor_left },
    { .key =     KEY_S, .func =       motor_backward },
    { .key =     KEY_D, .func =          motor_right },
    { .key = KEY_SPACE, .func = do_image_recognition },
    { .key =     KEY_R, .func =                 stop },
    { .key = KEY_E, .func = move_servo_forward },
    { .key = KEY_Q, .func = move_servo_backward },
};

int main(void)
{
    gpio_init();
    pca9685_init();
    input_init();
    int servo_pins[] = {9, 10, 11, 12, 13, 14, 15};
    sth = servo_thread_init(servo_pins);
    if(thrd_create(&img_thrd, img_thread, NULL) != thrd_success) {
        fprintf(stderr, "thrd_create elbukott\n");
    }
    printf("Up and running\n");
    while(1) { // robot loop
        input_receive_input();

        //for(int i = 30; i < 60; i++) {
        //    pca9685_pin_set(15, i);
        //    usleep(2000);
        //}
        //for(int i = 59; i >= 30; i--) {
        //    pca9685_pin_set(15, i);
        //    usleep(2000);
        //}

        //pwm_set_duty_cycle(r, 255);
        //pwm_set_duty_cycle(g, 255);
        //pwm_set_duty_cycle(b, 0);
        //sleep(1);
        //pwm_set_duty_cycle(r, 0);
        //pwm_set_duty_cycle(g, 255);
        //pwm_set_duty_cycle(b, 255);
        //sleep(1);
        //pwm_set_duty_cycle(r, 255);
        //pwm_set_duty_cycle(g, 255);
        //pwm_set_duty_cycle(b, 255);
        //sleep(1);
    }

    thrd_join(img_thrd, NULL);
}
