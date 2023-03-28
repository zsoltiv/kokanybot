#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <time.h>
#include <unistd.h>
#include <math.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>
#include <i2c/smbus.h>

#include "pca9685.h"

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

int pca9685;

static inline uint8_t pin(int n)
{
    // 0x06-nal kezdodnek, 0x04 octet kulonbseg van a LEDn_ON_LOW-ok kozott
    return (uint8_t) (0x06 + n * 0x04);
}

static inline uint16_t deg_to_pwm(int deg)
{
    // [0;179]-et konvertal a [0;4095] tartomanyba
    //return (deg - 0) * (4095 - 0) / (359 - 0) + 0;
    return (uint16_t) (deg * 4095u / 179u);
}

static uint8_t calculate_prescale(int freq)
{
    return (uint8_t) (round(PCA9685_OSCILLIATOR / (freq * 4096)) - 1);
}

void pca9685_init(void)
{
    pca9685 = open("/dev/i2c-1", O_RDWR);
    if(pca9685 < 0) {
        perror("open()");
    }

    if(ioctl(pca9685, I2C_SLAVE, PCA9685_ADDR)) {
        perror("ioctl()");
    }

    i2c_smbus_write_byte_data(pca9685, PCA9685_MODE1, MODE1_SLEEP);
    i2c_smbus_write_byte_data(pca9685, PCA9685_PRESCALE, calculate_prescale(50));
    nanosleep(&(struct timespec) {.tv_nsec = 500000}, NULL);
    i2c_smbus_write_byte_data(pca9685, PCA9685_MODE1, 0x1);
}

void pca9685_cleanup(void)
{
    close(pca9685);
}

void pca9685_pin_set(int n, int degrees)
{
    if(n < 0 || n > 15)
        return;
    if(degrees < 0 || degrees > 179)
        return;

    uint8_t p = pin(n);
    uint16_t pwm = deg_to_pwm(degrees);
    if(i2c_smbus_write_byte_data(pca9685, p + 0, pwm & 0xFF) < 0 ||
       i2c_smbus_write_byte_data(pca9685, p + 1, pwm >> 8) < 0 ||
       i2c_smbus_write_byte_data(pca9685, p + 2, 0) < 0 ||
       i2c_smbus_write_byte_data(pca9685, p + 3, 0) < 0)
        perror("i2c_smbus_write_word_data()");
}