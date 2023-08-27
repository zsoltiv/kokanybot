/*
 * copyright (c) 2023 Zsolt Vadasz
 *
 * This file is part of kokanybot.
 *
 * kokanybot is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License,
 * or (at your option) any later version.
 *
 * kokanybot is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with kokanybot. If not, see <https://www.gnu.org/licenses/>. 
*/

#define _XOPEN_SOURCE 700
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>

#include <i2c/smbus.h>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>

#include "i2c.h"
#include "mcp23017.h"

#define LOW_BYTE(x)  ((uint8_t)((x) & 0x00FF))
#define HIGH_BYTE(x) ((uint8_t)((x) >> 8))

struct mcp23017 {
    unsigned addr;
    uint16_t pins;
    bool active;
};

// relevant registers
enum mcp23017_reg {
    // IOCON.BANK = 0
    IODIRA = 0x00,
    IODIRB = 0x01,
    IPOLA  = 0x02,
    IPOLB  = 0x03,
    GPIOA  = 0x12,
    GPIOB  = 0x13,
};

struct mcp23017 *mcp23017_init(unsigned addr)
{
    struct mcp23017 *ic = malloc(sizeof(struct mcp23017));
    ic->addr = addr;
    ic->active = true;

    if(ioctl(i2c, I2C_SLAVE, ic->addr) < 0)
        perror("ioctl()");

    // set all GPIOs as outputs
    i2c_smbus_write_byte_data(i2c, IODIRA, 0x0);
    i2c_smbus_write_byte_data(i2c, IODIRB, 0x0);
    i2c_smbus_write_byte_data(i2c, IPOLA, 0x0);
    i2c_smbus_write_byte_data(i2c, IPOLB, 0x0);
    ic->pins = 0;
    i2c_smbus_write_byte_data(i2c, GPIOA, LOW_BYTE(ic->pins));
    i2c_smbus_write_byte_data(i2c, GPIOB, HIGH_BYTE(ic->pins));

    return ic;
}

void mcp23017_set(struct mcp23017 *ic, unsigned pin, bool value)
{
    ic->pins = (ic->pins & ~(1u << pin)) | (value << pin);
    if(i2c_smbus_write_byte_data(i2c, GPIOA, LOW_BYTE(ic->pins)) < 0)
        perror("i2c_smbus_write_byte_data");
    if(i2c_smbus_write_byte_data(i2c, GPIOB, HIGH_BYTE(ic->pins)) < 0)
        perror("i2c_smbus_write_byte_data");
}
