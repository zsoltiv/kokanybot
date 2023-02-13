#ifndef INPUT_H
#define INPUT_H

#include <stdint.h>
#include <stdbool.h>

struct key_state {
    uint32_t key;
    bool pressed;
};

// WASD allapotok GPIO-hoz
extern struct key_state key_states[4];

void input_init(void);
void input_close(void);
void input_receive_input(void);

#endif /* INPUT_H */
