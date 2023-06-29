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

#ifndef MQ135_H
#define MQ135_H

#ifdef __cplusplus
extern "C" {
#endif

#include <sys/types.h>

struct mq135;

struct mq135 *mq135_init(struct sockaddr *sa, unsigned port, unsigned pin);
int mq135_thread(void *arg);
bool mq135_get_presence(struct mq135 *sensor);

#ifdef __cplusplus
}
#endif

#endif /* MQ135_H */
