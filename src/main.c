#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <linux/input-event-codes.h>

#include "img.h"
#include "i2c.h"
#include "gpio.h"
#include "input.h"

struct key_bind key_binds[4] = {
    { .key = KEY_W, .func = motor_forward  },
    { .key = KEY_A, .func = motor_left     },
    { .key = KEY_S, .func = motor_backward },
    { .key = KEY_D, .func = motor_right    },
};

int main(void)
{
    input_init();
    while(1) { // robot loop
        input_receive_input();
    }
}
