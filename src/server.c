#include <gpiod.h>

#include <stdio.h>

#include "gpio.h"

int main()
{
    gpio_init();

    motor_forward();
    while(true);

    gpio_cleanup();
    return 0;
}
