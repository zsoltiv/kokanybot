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

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>

#include <i2c/smbus.h>
#include <linux/i2c-dev.h>

#include "i2c.h"

int i2c;

void i2c_init(void)
{
    i2c = open("/dev/i2c-1", O_RDWR);
    if(i2c < 0)
        perror("open()");
}
