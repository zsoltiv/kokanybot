#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdbool.h>
#include <errno.h>
#include <linux/input-event-codes.h>

#include "net.h"
#include "input.h"

static int get_key_bind_index(uint8_t key)
{
    for(int i = 0; i < sizeof(key_binds) / sizeof(key_binds[0]); i++) {
        if(key_binds[i].key == key)
            return i;
    }

    return -1;
}

static bool is_key_pressed(uint8_t keycode)
{
    // highest bit indicates whether the key has been pressed
    return keycode & 0x80;
}

void input_init(void)
{
    for(int i = 0; i < sizeof(key_binds) / sizeof(key_binds[0]); i++)
        key_binds[i].prev_state = false;
}

void input_process_key_event(uint8_t keycode)
{
    int state_idx = get_key_bind_index(keycode);
    if(state_idx > -1) {
        bool key_state = is_key_pressed(keycode);
        if(key_state != key_binds[state_idx].prev_state) {
            key_binds[state_idx].func(key_state);
            key_binds[state_idx].prev_state = key_state;
        }
    } else
        fprintf(stderr, "rossz gomb\n");
}
