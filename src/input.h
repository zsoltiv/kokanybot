#ifndef INPUT_H
#define INPUT_H

#include <stdint.h>
#include <stdbool.h>

typedef void (*toggle_func)(bool);

struct key_bind {
    uint32_t key;
    toggle_func func;
};

// WASD allapotok GPIO-hoz
extern struct key_bind key_binds[6];

void input_init(void);
void input_close(void);
void input_receive_input(void);

#endif /* INPUT_H */
