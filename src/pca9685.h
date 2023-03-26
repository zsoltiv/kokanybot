#ifndef PCA9685_H
#define PCA9685_H

#ifdef __cplusplus
extern "C" {
#endif

void pca9685_init(void);
void pca9685_cleanup(void);
void pca9685_pin_set(int n, int degrees);


#ifdef __cplusplus
}
#endif

#endif /* PCA9685_H */
