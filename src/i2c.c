#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>

#include "i2c.h"

/*
 * PCA9685 datasheet
 * https://pdf1.alldatasheet.com/datasheet-pdf/view/293576/NXP/PCA9685.html
 */

#define PCA9685_ADDR 0x40

int i2c;

static inline uint8_t servo(int n)
{
    // 0x6-nal kezdodnek, 4 octet kulonbseg van a LEDn_ON_LOW-ok kozott
    return (uint8_t) (n * 4 + 6);
}

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

void i2c_servo_set(int n, int degrees)
{
    if(n < 0 || n > 15)
        return;
    if(degrees < 0 || degrees > 359)
        return;

    uint8_t addr = servo(n);
    write(i2c, &addr, 1);
}
