#include <stdio.h>
#include <stdlib.h>
#include <threads.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <linux/input-event-codes.h>

#include "img.h"
#include "i2c.h"
#include "gpio.h"
#include "input.h"

struct key_bind key_binds[5] = {
    { .key =     KEY_W, .func =        motor_forward },
    { .key =     KEY_A, .func =           motor_left },
    { .key =     KEY_S, .func =       motor_backward },
    { .key =     KEY_D, .func =          motor_right },
    { .key = KEY_SPACE, .func = do_image_recognition },
};

int main(void)
{
    gpio_init();
    input_init();
    if(thrd_create(&img_thrd, img_thread, NULL) != thrd_success) {
        fprintf(stderr, "thrd_create elbukott\n");
    }
    while(1) { // robot loop
        input_receive_input();
    }

    thrd_join(img_thrd, NULL);
}
