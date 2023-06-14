#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>

#include <i2c/smbus.h>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>

#include "i2c.h"
#include "mcp23017.h"

#define LOW_BYTE(x) ((uint8_t)((x) & 0x00FF))
#define HIGH_BYTE(x) ((uint8_t)((x) & 0xFF00))

struct mcp23017 {
    unsigned addr;
    uint16_t pins;
    bool active;
};

// relevant registers
enum mcp23017_reg {
    IODIRA = 0x0u,
    IODIRB = 0x1u,
    IPOLA = 0x2u,
    IPOLB = 0x3u,
    IOCON_LOW = 0x0Au,
    IOCON_HIGH = 0x0Bu,
    GPIOA = 0x12u,
    GPIOB = 0x13u,
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
    i2c_smbus_write_byte_data(i2c, GPIOA, HIGH_BYTE(ic->pins));

    return ic;
}

void mcp23017_set(struct mcp23017 *ic, unsigned pin, bool value)
{
    ic->pins = (ic->pins & ~(1u << pin)) | (value << pin);
    i2c_smbus_write_byte_data(i2c, GPIOA, LOW_BYTE(ic->pins));
    i2c_smbus_write_byte_data(i2c, GPIOA, HIGH_BYTE(ic->pins));
}
