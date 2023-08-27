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

#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdbool.h>
#include <errno.h>

#include "net.h"
#include "input.h"

static bool is_key_pressed(uint8_t keycode)
{
    // highest bit indicates whether the key has been pressed
    return !!(keycode & 0x80);
}

void input_init(void)
{
    for(int i = 0; i < sizeof(key_binds) / sizeof(key_binds[0]); i++)
        key_binds[i].prev_state = false;
}

void input_process_key_event(uint8_t keycode)
{
    bool key_state = is_key_pressed(keycode);
    keycode &= 0x7F; // discard MSB
    if(key_state != key_binds[keycode].prev_state) {
        key_binds[keycode].func(key_state);
        key_binds[keycode].prev_state = key_state;
    }
}
