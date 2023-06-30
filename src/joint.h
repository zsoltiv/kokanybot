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

#ifndef JOINT_H
#define JOINT_H

enum {
    JOINT_STILL,
    JOINT_FORWARD,
    JOINT_BACKWARD,
};

struct arm;

struct arm *arm_init(void);
void arm_select_joint(struct arm *arm, int joint);
void arm_set_dir(struct arm *arm, int dir);

#endif /* JOINT_H */
