/* Clue: minimal C-Lua integration
 *
 * Runtime code
 *
 * Copyright (c) 2007, 2009, 2010, 2011 Reuben Thomas.
 *
 ** Lua stand-alone interpreter adapted from lua.c in Lua distribution:
 ** $Id: lua.c,v 1.160.1.2 2007/12/28 15:32:23 roberto Exp $
 **
 ******************************************************************************
 * Copyright (C) 1994-2008 Lua.org, PUC-Rio.  All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 ******************************************************************************/

#include <assert.h>
#include <signal.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#include "clue.h"

#define lua_c

#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

clue_State *clue_init (void)
{
  clue_State *L;

  if ((L = luaL_newstate()))
    luaL_openlibs(L);
  return L;
}

void clue_close (clue_State *L)
{
  lua_close (L);
}

void clue_do (clue_State *L, const char *code)
{
  assert (luaL_loadstring(L, code) == 0);
  clue_docall (L, 0, 1);
}


static clue_State *globalL;


static void lstop (lua_State *L, lua_Debug *ar) {
  (void)ar;  /* unused arg. */
  lua_sethook(L, NULL, 0, 0);
  luaL_error(L, "interrupted!");
}


static void laction (int i) {
  signal(i, SIG_DFL); /* if another SIGINT happens before lstop,
                         terminate process (default action) */
  lua_sethook(globalL, lstop, LUA_MASKCALL | LUA_MASKRET | LUA_MASKCOUNT, 1);
}


static void l_message (const char *msg) {
  fprintf(stderr, "%s\n", msg);
  fflush(stderr);
}


static int report (lua_State *L, int status) {
  if (status && !lua_isnil(L, -1)) {
    const char *msg = lua_tostring(L, -1);
    if (msg == NULL) msg = "(error object is not a string)";
    l_message(msg);
    lua_pop(L, 1);
  }
  return status;
}


static int traceback (lua_State *L) {
  if (!lua_isstring(L, 1))  /* 'message' not a string? */
    return 1;  /* keep it intact */
  lua_getfield(L, LUA_GLOBALSINDEX, "debug");
  if (!lua_istable(L, -1)) {
    lua_pop(L, 1);
    return 1;
  }
  lua_getfield(L, -1, "traceback");
  if (!lua_isfunction(L, -1)) {
    lua_pop(L, 2);
    return 1;
  }
  lua_pushvalue(L, 1);  /* pass error message */
  lua_pushinteger(L, 2);  /* skip this function and traceback */
  lua_call(L, 2, 1);  /* call debug.traceback */
  return 1;
}


static int docall (lua_State *L, int narg, int clear) {
  int status;
  globalL = L;
  int base = lua_gettop(L) - narg;  /* function index */
  lua_pushcfunction(L, traceback);  /* push traceback function */
  lua_insert(L, base);  /* put it under chunk and args */
  signal(SIGINT, laction);
  status = lua_pcall(L, narg, (clear ? 0 : LUA_MULTRET), base);
  signal(SIGINT, SIG_DFL);
  lua_remove(L, base);  /* remove traceback function */
  /* force a complete garbage collection in case of errors */
  if (status != 0) lua_gc(L, LUA_GCCOLLECT, 0);
  return status;
}


void clue_docall (lua_State *L, int narg, int clear) {
  int status = docall (L, narg, clear);
  if (status) {
    report(L, status);
    raise(SIGABRT);
  }
}

int clue_call_va (clue_State *L, const char *func, const char *sig, ...) {
  va_list vl;
  int status;
  int narg, nres;   /* number of arguments and results */

  va_start(vl, sig);
  lua_getglobal(L, func);  /* get function */

  /* push arguments */
  narg = 0;
  while (*sig) {    /* push arguments */
    switch (*sig++) {
    case 'n':
      lua_pushnumber(L, va_arg(vl, lua_Number));
      break;
    case 'i':
      lua_pushinteger(L, va_arg(vl, lua_Integer));
      break;
    case 'b':
      lua_pushboolean(L, va_arg(vl, int));
      break;
    case 's':
      lua_pushstring(L, va_arg(vl, const char *));
      break;
    case 'u':
      lua_pushlightuserdata(L, va_arg(vl, void *));
      break;
    case 'f':
      lua_pushcfunction(L, va_arg(vl, lua_CFunction));
      break;
    case '>':
      goto endwhile;
    default:
      luaL_error(L, "invalid option (%c)", *(sig - 1));
    }
    narg++;
    luaL_checkstack(L, 1, "too many arguments");
  } endwhile:

  /* do the call */
  nres = (int)strlen(sig);  /* number of expected results */
  status = lua_pcall(L, narg, nres, 0);
  if (status != 0)  /* do the call */
    luaL_error(L, "error running function `%s': %s",
               func, lua_tostring(L, -1));
  /* retrieve results */
  nres = -nres;     /* stack index of first result */
  while (*sig) {    /* get results */
    switch (*sig++) {
    case 'n':
      if (!lua_isnumber(L, nres))
        luaL_error(L, "wrong result type");
      *va_arg(vl, lua_Number *) = lua_tonumber(L, nres);
      break;
    case 'i':
      if (!lua_isnumber(L, nres))
        luaL_error(L, "wrong result type");
      *va_arg(vl, lua_Integer *) = lua_tointeger(L, nres);
      break;
    case 'b':
      if (!lua_isboolean(L,nres))
        luaL_error(L, "wrong result type");
      *va_arg(vl, int *) = (int)lua_toboolean(L, nres);
      break;
    case 's':
      if (!lua_isstring(L, nres))
        luaL_error(L, "wrong result type");
      *va_arg(vl, const char **) = lua_tostring(L, nres);
      break;
    case 'u':
      if (!lua_islightuserdata(L,nres))
        luaL_error(L, "wrong result type");
      *va_arg(vl, void **) = (void*)lua_topointer(L, nres);
      break;
    default:
      luaL_error(L, "invalid option (%c)", *(sig - 1));
    }
    nres++;
  }
  va_end(vl);
  return status;
}
