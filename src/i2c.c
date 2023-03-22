#include <unistd.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>
#include <i2c/smbus.h>

#include "i2c.h"

/*
 * PCA9685 datasheet
 * https://pdf1.alldatasheet.com/datasheet-pdf/view/293576/NXP/PCA9685.html
 */

#define PCA9685_ADDR 0x40
#define PCA9685_MODE1 0x0
#define PCA9685_PRESCALE 0xFE
// 25Mhz
#define PCA9685_OSCILLIATOR 25000000u
#define MODE1_SLEEP (1u << 3)
#define MODE1_RESTART (1u << 7)

int i2c;

static inline uint8_t servo(int n)
{
    // 0x06-nal kezdodnek, 0x04 octet kulonbseg van a LEDn_ON_LOW-ok kozott
    return (uint8_t) (0x06 + n * 0x04);
}

static inline uint16_t deg_to_pwm(int deg)
{
    // [0;359]-et konvertal a [0;4095] tartomanyba
    //return (deg - 0) * (4095 - 0) / (359 - 0) + 0;
    return (uint16_t) (deg * 4095u / 359u);
}

static uint8_t calculate_prescale(int freq)
{
    return (uint8_t) (round(PCA9685_OSCILLIATOR / (freq * 4096)) - 1);
}

void i2c_init(void)
{
    i2c = open("/dev/i2c-1", O_RDWR);
    if(i2c < 0) {
        perror("open()");
        i2c_cleanup();
    }

    if(ioctl(i2c, I2C_SLAVE, PCA9685_ADDR)) {
        perror("ioctl()");
        i2c_cleanup();
    }

    i2c_smbus_write_byte_data(i2c, PCA9685_MODE1, MODE1_SLEEP);
    i2c_smbus_write_byte_data(i2c, PCA9685_PRESCALE, calculate_prescale(50));
    usleep(500);
    i2c_smbus_write_byte_data(i2c, PCA9685_MODE1, 0x1);
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

    uint8_t s = servo(n);
    uint16_t pwm = deg_to_pwm(degrees);
    if(i2c_smbus_write_byte_data(i2c, servo(n) + 0, pwm & 0xFF) < 0 ||
       i2c_smbus_write_byte_data(i2c, servo(n) + 1, pwm >> 8) < 0 ||
       i2c_smbus_write_byte_data(i2c, servo(n) + 2, 0) < 0 ||
       i2c_smbus_write_byte_data(i2c, servo(n) + 3, 0) < 0)
        perror("i2c_smbus_write_word_data()");
}
