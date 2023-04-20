#ifndef INPUT_H
#define INPUT_H

#include <libinput.h>

#include <stdint.h>
#include <stdbool.h>

#define INPUT_KEY_BINDS 13

typedef void (*toggle_func)(bool);

struct key_bind {
    uint8_t key;
    toggle_func func;
    bool prev_state;
};

// WASD allapotok GPIO-hoz
extern struct key_bind key_binds[INPUT_KEY_BINDS];

void input_init(void);
void input_process_key_event(uint8_t keycode);

#endif /* INPUT_H */
