#ifndef I2C_H
#define I2C_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

/* 
 * PCA9685 relevans registerei
 *
 * https://pdf1.alldatasheet.com/datasheet-pdf/view/293576/NXP/PCA9685.html
 */

#define SERVO(n) ((uint8_t) ((n) * 4 + 6)) // 0x6-nal kezdodnek, 4 octet kulonbseg van a LEDn_ON_LOW-ok kozott

void i2c_init(void);
void i2c_cleanup(void);

extern int i2c;

#ifdef __cplusplus
}
#endif

#endif /* I2C_H */
