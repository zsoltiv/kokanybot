#ifndef INPUT_H
#define INPUT_H

#include <libinput.h>

#include <stdint.h>
#include <stdbool.h>

#define INPUT_KEY_BINDS 13

typedef void (*toggle_func)(bool);

struct key_bind {
    uint32_t key;
    toggle_func func;
    enum libinput_key_state prev_state;
};

// WASD allapotok GPIO-hoz
extern struct key_bind key_binds[INPUT_KEY_BINDS];

void input_init(void);
void input_close(void);
void input_receive_input(void);
void dispatch_pending_events(void);

#endif /* INPUT_H */
