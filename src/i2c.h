#ifndef I2C_H
#define I2C_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

void i2c_init(void);
void i2c_cleanup(void);

extern int i2c;

#ifdef __cplusplus
}
#endif

#endif /* I2C_H */
