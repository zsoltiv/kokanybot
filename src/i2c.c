#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>

#include "i2c.h"

#define PCA9685_ADDR 0x40

int i2c;

void i2c_init(void)
{
    i2c = open("/dev/i2c-1", O_RDWR);
    if(i2c < 0) {
        perror("open()");
        i2c_cleanup();
        exit(1);
    }

    if(ioctl(i2c, I2C_SLAVE, PCA9685_ADDR)) {
        perror("ioctl()");
        i2c_cleanup();
        exit(1);
    }
}

void i2c_cleanup(void)
{
    close(i2c);
}
