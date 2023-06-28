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

#ifndef MCP23017_H
#define MCP23017_H

#ifdef __cplusplus
extern "C" {
#endif

struct mcp23017;

struct mcp23017 *mcp23017_init(unsigned addr);
void mcp23017_set(struct mcp23017 *ic, unsigned pin, bool value);

#ifdef __cplusplus
}
#endif

#endif /* MCP23017_H */
