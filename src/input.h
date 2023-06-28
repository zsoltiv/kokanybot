/*
 * copyright (c) 2023 Zsolt Vadasz
 *
 * This file is part of kokanybot.
 *
 * kokanybot is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License,
 * or (at your option) any later version.
 *
 * kokanybot is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with kokanybot. If not, see <https://www.gnu.org/licenses/>. 
*/

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
