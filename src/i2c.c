#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>

#include <i2c/smbus.h>
#include <linux/i2c-dev.h>

#include "i2c.h"

int i2c;

void i2c_init(void)
{
    i2c = open("/dev/i2c-1", O_RDWR);
    if(i2c < 0)
        perror("open()");
}
