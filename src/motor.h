#ifndef MOTOR_H
#define MOTOR_H

#ifdef __cplusplus
extern "C" {
#endif

void motor_init(void);
void motor_cleanup(void);
void motor_forward(bool pressed);
void motor_backward(bool pressed);
void motor_left(bool pressed);
void motor_right(bool pressed);
void motor_stop(bool pressed);

#ifdef __cplusplus
}
#endif

#endif /* MOTOR_H */
