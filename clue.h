/* Clue: minimal C-Lua integration

   Header file

   Copyright (c) 2007, 2009, 2010, 2011 Reuben Thomas.

   Permission is hereby granted, free of charge, to any person
   obtaining a copy of this software and associated documentation
   files (the "Software"), to deal in the Software without
   restriction, including without limitation the rights to use, copy,
   modify, merge, publish, distribute, sublicense, and/or sell copies
   of the Software, and to permit persons to whom the Software is
   furnished to do so, subject to the following conditions:

   The above copyright notice and this permission notice shall be
   included in all copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
   EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
   MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
   NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
   BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
   ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
   CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
   SOFTWARE. */

#include <assert.h>

#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>


/* Public API */

typedef lua_State clue_State;

clue_State *clue_init (void);
void clue_close (clue_State *L);
void clue_do (clue_State *L, const char *code);
int clue_call_va (clue_State *L, const char *func, const char *sig, ...);

#define clue_pdo(L, code)                       \
  (assert (luaL_loadstring(L, code) == 0), lua_pcall(L, 0, 0, 0))

#define CLUE_SET(L, lvar, lty, cval)            \
  do {                                          \
    lua_checkstack(L, 1);                       \
    lua_push ## lty(L, cval);                   \
    lua_setglobal(L, #lvar);                    \
  } while (0)

#define CLUE_GET(L, lvar, lty, cvar)            \
  do {                                          \
    lua_checkstack(L, 1);                       \
    lua_getglobal(L, #lvar);                    \
    cvar = lua_to ## lty(L, -1);                \
    lua_pop(L, 1);                              \
  } while (0)


/* Private API */

void clue_docall (lua_State *L, int narg, int clear);
