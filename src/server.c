#include <gpiod.h>

#include <signal.h>
#include <stdio.h>

#include "sig.h"
#include "gpio.h"

int main()
{
    signal(SIGINT, sig_handler);
    signal(SIGHUP, sig_handler);
    gpio_init();

    motor_forward();
    while(true);

    gpio_cleanup();
    return 0;
}
