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
