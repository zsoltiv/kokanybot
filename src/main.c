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
#include "pwm.h"

void stop(bool unused)
{
    dispatch_pending_events();
    motor_stop(unused);
}

struct key_bind key_binds[6] = {
    { .key =     KEY_W, .func =        motor_forward },
    { .key =     KEY_A, .func =           motor_left },
    { .key =     KEY_S, .func =       motor_backward },
    { .key =     KEY_D, .func =          motor_right },
    { .key = KEY_SPACE, .func = do_image_recognition },
    { .key =     KEY_R, .func =                 stop },
};

int main(void)
{
    gpio_init();
    pca9685_init();
    input_init();
    if(thrd_create(&img_thrd, img_thread, NULL) != thrd_success) {
        fprintf(stderr, "thrd_create elbukott\n");
    }
    printf("Up and running\n");
    while(1) { // robot loop
        //input_receive_input();

        //for(int i = 0; i < 180; i++) {
        //    pca9685_pin_set(15, i);
        //    usleep(1000);
        //}
        //for(int i = 179; i >= 0; i--) {
        //    pca9685_pin_set(15, i);
        //    usleep(1000);
        //}

        pwm_set_duty_cycle(r, 255);
        pwm_set_duty_cycle(g, 255);
        pwm_set_duty_cycle(b, 0);
        sleep(1);
        pwm_set_duty_cycle(r, 0);
        pwm_set_duty_cycle(g, 255);
        pwm_set_duty_cycle(b, 255);
        sleep(1);
        pwm_set_duty_cycle(r, 255);
        pwm_set_duty_cycle(g, 255);
        pwm_set_duty_cycle(b, 255);
        sleep(1);
    }

    thrd_join(img_thrd, NULL);
}
