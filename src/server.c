#include <gpiod.h>

#include <stdio.h>

#include "gpio.h"

int main()
{
    gpio_init();

    printf("works as intended\n");

    gpio_cleanup();
    return 0;
}
