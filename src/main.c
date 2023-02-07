#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/ioctl.h>

#include "img.h"
#include "i2c.h"
#include "input.h"

int main(void)
{
    input_init();
    i2c_init();
    i2c_servo_set(0, 359);
    while(1) { // robot loop
        // TODO input
        sleep(1);
    }
}
