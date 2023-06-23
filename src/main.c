#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <threads.h>
#include <time.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <linux/input-event-codes.h>

#include "net.h"
#include "gpio.h"
#include "i2c.h"
#include "motor.h"
#include "input.h"

struct key_bind key_binds[INPUT_KEY_BINDS] = {
    { .key =     KEY_W, .func =        motor_forward },
    { .key =     KEY_A, .func =           motor_left },
    { .key =     KEY_S, .func =       motor_backward },
    { .key =     KEY_D, .func =          motor_right },
};

int main(void)
{
    input_init();
    struct sockaddr_in inaddr;
    net_get_interface_addr((struct sockaddr *)&inaddr);
    int listener = net_listener_new((struct sockaddr *)&inaddr, 1337);
    int client = net_accept(listener);
    gpio_init();
    i2c_init();
    printf("Up and running\n");
    while(1) {
        uint8_t keycode = net_receive_keypress(client);
        input_process_key_event(keycode);
    }
}
