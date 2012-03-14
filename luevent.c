/**
 * @file
 * Lua binding to libuevent
 * $Id:$
 *
 * (C) Copyright 2012 flemming.madsen at madsensoft.dk. See libuevent.h
 */
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <stdbool.h>
#include <assert.h>
#include <lua.h>
#include <lauxlib.h>
#include "libuevent.h"

static char * LUA_LTIMER_MT  = "libuevent-timer";
static char * LUA_LFILEH_MT  = "libuevent-fileh";
static char * LUA_LUEOBJ  = "libuevent-objects";

typedef struct lue_data
{
   lua_State     *L;            // Index to our world
   struct LUETimerH *timer;
   struct LUEFileEventH *file;
} lue_data;
#define toptimer(L)  ((struct lue_data *)luaL_checkudata(L, 1, LUA_LTIMER_MT))
#define topfileh(L)  ((struct lue_data *)luaL_checkudata(L, 1, LUA_LFILEH_MT))

static struct LUECtxt *getCtxt(lua_State *L)
{
   lua_getfield(L, LUA_REGISTRYINDEX, LUA_LUEOBJ);
   lua_rawgeti(L, -1, 1);
   struct LUECtxt *ret = lua_touserdata(L, -1);
   lua_pop(L, 2);
   return ret;
}

static void l_newType(lua_State* L, const char *name)
{
   luaL_newmetatable(L, name);       // create metatable
   lua_pushvalue(L, -1);             // dup
   lua_setfield(L, -2, "__index");   // metatable.__index = metatable
}

static int l_traceback (lua_State *L)
{
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

// Top half
static int _lueHandler(lua_State *L, void *data, bool remObj)
{
   lua_pushcfunction(L, l_traceback);  /* push traceback function */

   lua_getfield(L, LUA_REGISTRYINDEX, LUA_LUEOBJ);
   lua_pushlightuserdata(L, data);
   lua_rawget(L, -2);                    // objects[object_p] => The object
   lua_pushvalue(L, -1);
   lua_rawget(L, -3);                    // object[object] => closure

   if (remObj)
   {
      lua_pushlightuserdata(L, data);
      lua_pushnil(L);
      lua_rawset(L, -5);                 // objects[object_p] = nil -> GC
      lua_pushvalue(L, -2);
      lua_pushnil(L);
      lua_rawset(L, -5);                 // objects[object] = nil -> GC
   }
   lua_remove(L, -3);                    // objects table

   int ret;
   lua_insert(L, -2);                    // exch: closure <-> obj
   if ((ret = lua_pcall(L, 1, 0, -3)))   // Call closure(object)
   {
      fprintf(stderr, "luevent: Error: %d:\n%s\n",
                      ret, lua_isnil(L, -1) ? "(nil)" : lua_tostring(L, -1));
      //exit(1);
   }
   lua_pop(L, 1);                        // Error handler, metatable

   return 0;
}

static int _ltimerHandler(struct LUECtxt *ctxt, struct LUETimerH *handle, void *data)
{
   lue_data *self = (lue_data *) data;
   if (!self || !self->timer)
      return 0;

   int ret = _lueHandler(self->L, data, true);
   self->timer = NULL;
   return ret;
}

static int _lfileHandler(struct LUECtxt *ctxt, struct LUEFileEventH *handle, int fd, void *data)
{
   lue_data *self = (lue_data *) data;
   bool remObj = false;
   if (!self || !self->file)
      return 0;

   if (fcntl(fd, F_GETFD) == -1 && EBADF == errno)
   {
      remObj = true;
      lueRemInput(getCtxt(self->L), self->file); // Works on both types
      self->file = NULL;
   }

   return _lueHandler(self->L, data, remObj);
}

// Bottom half
static void lueAddObj(lua_State *L, lue_data *self, int cbix)
{
   self->L = L;
   lua_getfield(L, LUA_REGISTRYINDEX, LUA_LUEOBJ);

   lua_pushlightuserdata(L, self);
   lua_pushvalue(L, -3);
   lua_rawset(L, -3);                    // objects[self_p] = obj

   lua_pushvalue(L, -2);
   lua_pushvalue(L, cbix);
   lua_rawset(L, -3);                    // objects[obj] = callback
   lua_pop(L, 1);                        // objects
}

 /* LUADOC
  * Add a handler on a stream.
  * The handler will sustain itself GC wise. Use &lt:object>:clear() to unregister the handler
  * @param fdesc The posix file descriptor "small integer".
  *   <br> Use the ":getfd()" method on a "normal" lua stream or socket to obtain fdesc.
  * @param handler A function object. No paramaeter supplied when called.
  * @param mode [Optional] Input ("r") or output ("w") event. Default is input
  * @return A new filehandler object
  function luevent.addFileIO(fdesc, handler, mode)
  */
static int lfileh_add(lua_State *L)
{
   int fd = luaL_checkint(L, 1);
   luaL_checktype(L, 2, LUA_TFUNCTION);
   const char *mode = luaL_optstring(L, 3, "r");

   lue_data *self = (lue_data *) lua_newuserdata(L, sizeof(lue_data));
   struct LUEFileEventH *h = *mode == 'r' ?
                              lueAddInput(getCtxt(L), fd, _lfileHandler, self) :
                              lueAddOutput(getCtxt(L), fd, _lfileHandler, self);
   if (!h) luaL_error(L, "lueAddFileIO error");
   self->file = h;

   luaL_getmetatable(L, LUA_LFILEH_MT);
   lua_setmetatable(L, -2);
   lueAddObj(L, self, 2);

   return 1;
}

 /* LUADOC
  * Create a new timer object. Implicitly cleared when handler is called.
  * @usage <pre>
  *  local hdl<br>
  *  hdl = luevent.addTimer(50, function() hdl = nil xxx end)<br>
  *  ......<br>
  *  if hdl then hdl:clear() hdl = nil end<br>
  * </pre>
  * Here <tt>hdl</tt> is used to keep track of whether the timer is active.
  * @param msecs The number of milliseconds until the timer expires
  * @param handler A function object 
  * @return A new timer object
  function luevent.addTimer(msecs, handler)
  */
static int ltimer_add(lua_State *L)
{
   int mSecs = luaL_checkint(L, 1);
   luaL_checktype(L, 2, LUA_TFUNCTION);

   lue_data *self = (lue_data *) lua_newuserdata(L, sizeof(lue_data));
   struct LUETimerH *h = self ? lueAddTimer(getCtxt(L), mSecs, _ltimerHandler, self) : NULL;
   if (!h)
      luaL_error(L, "lueAddTimer error");
   self->timer = h;

   luaL_getmetatable(L, LUA_LTIMER_MT);
   lua_setmetatable(L, 3);
   lueAddObj(L, self, 2);

   return 1;
}

static void lobj_clear(lua_State *L, lue_data *self)
{
   lua_getfield(L, LUA_REGISTRYINDEX, LUA_LUEOBJ);
   lua_pushlightuserdata(L, self);
   lua_rawget(L, -2);                 // objects[object_p] => The object

   lua_pushnil(L);
   lua_rawset(L, -3);                 // objects[object] = nil -> GC
   lua_pushlightuserdata(L, self);
   lua_pushnil(L);
   lua_rawset(L, -3);                 // objects[object_p] = nil -> GC
   lua_pop(L, 1);                     // pop objects
}

 /* LUADOC
  * Cancel a timer callback. Renders the object ready for GC.
  function timer:clear()
  */
static int ltimer_clear(lua_State *L)
{
   lue_data *self = toptimer(L);

   if (self->timer)
   {
      lueRemTimer(getCtxt(L), self->timer);
      self->timer = NULL;
   }
   lobj_clear(L, self);

   return 0;
}

 /* LUADOC
  * Cancel a fileio callback. Renders the object ready for GC.
  function fileio:clear()
  */
static int lfileh_clear(lua_State *L)
{
   lue_data *self = topfileh(L);

   if (self->file)
   {
      lueRemInput(getCtxt(L), self->file); // Works on both types
      self->file = NULL;
   }
   lobj_clear(L, self);

   return 0;
}

 /* LUADOC
  * Obtain the registered handler on the object
  * @return The handler
  function luevent_object:handler()
  */
static int luevent_handler(lua_State *L)
{
   lue_data *self = lua_touserdata(L, 1);

   if (self)
   {
      lua_getfield(L, LUA_REGISTRYINDEX, LUA_LUEOBJ);

      lua_getfield(L, -1, "_objects");
      lua_getfield(L, -2, "_callbacks");
      if (lua_istable(L, -1) && lua_istable(L, -2))
      {
         lua_pushlightuserdata(L, self);
         lua_rawget(L, -2);                    // The object
         lua_rawget(L, -2);                    // callbacks[obj] -> closure
         return 1;
      }
   }
   return 0;
}

static int ltimer_gc(lua_State *L)
{
   //STACKDUMP(L);
   lue_data *self = toptimer(L);

   if (self->timer)
   {
      lueRemTimer(getCtxt(L), self->timer);
      self->timer = NULL;
   }
   return 0;
}

static int lfileh_gc(lua_State *L)
{
   lue_data *self = topfileh(L);

   if (self->file)
   {
      lueRemInput(getCtxt(L), self->file);
      self->file = NULL;
   }
   return 0;
}

static int luevent_tostring(lua_State *L)
{
   lue_data *self = lua_touserdata(L, 1);

   lua_getmetatable(L, 1);
   lua_getfield(L, -1, "_typename");
   const char *t = lua_tostring(L, -1);

   if (!self->timer && !self->file)
      lua_pushfstring(L, "%s (cleared)", t);
   else
      lua_pushfstring(L, "%s (%p)", t, self);
   return 1;
}

/* LUADOC
 * Get the system time in hight resolution
 * @return two numbers: seconds and milliseconds since the epoch
 function luevent.systemTime()
 */
static int l_systemTime (lua_State *L)
{
   struct timeval v;

   gettimeofday(&v, (struct timezone *) NULL);
   lua_pushinteger(L, v.tv_sec);
   lua_pushinteger(L, v.tv_usec / 1000);
   return 2;  // num of results
}

static int l_run(lua_State * L)
{
   lueRun(getCtxt(L));
   return 0;
}

static int l_terminate(lua_State * L)
{
   lueTerminate(getCtxt(L));
   return 0;
}

static int l_cleanup(lua_State * L)
{
   struct LUECtxt *ctxt = getCtxt(L);

   lua_pushnil(L);
   lua_setfield(L, LUA_REGISTRYINDEX, LUA_LUEOBJ);
   lua_gc(L, LUA_GCCOLLECT, 0);  // Make sure all timers & schedules & callbacks are gone before context
 
   lueCtxtDestroy(ctxt);
   return 0;
}

static const luaL_Reg lueventlib[] = {
   {"addFileIO", lfileh_add},
   {"clearFileIO", lfileh_clear},
   {"addTimer", ltimer_add},
   {"clearTimer", ltimer_clear},
   {"systemTime", l_systemTime},
   {"run", l_run},
   {"terminate", l_terminate},
   {"__gc", l_cleanup},
  { NULL, NULL}
};

static const luaL_Reg ltimermeta[] = {
   {"clear", ltimer_clear},
   {"handler", luevent_handler},
   {"__gc", ltimer_gc},
   {"__tostring", luevent_tostring},
  { NULL, NULL}
};

static const luaL_Reg lfilehmeta[] = {
   {"clear", lfileh_clear},
   {"handler", luevent_handler},
   {"__gc", lfileh_gc},
   {"__tostring", luevent_tostring},
  { NULL, NULL}
};

int luaopen_luevent(lua_State * L)
{
   struct LUECtxt *lueContext = lueCtxtCreate();
   assert(lueContext);

   luaL_register(L, lua_tostring(L, 1), lueventlib); // calls
   lua_setmetatable(L, -1);            // Call GC at module cleanup. Pops it too

   l_newType(L, LUA_LTIMER_MT);        // create metatable for timers
   luaL_register(L, NULL, ltimermeta); // methods
   lua_pop(L, 1);

   l_newType(L, LUA_LFILEH_MT);        // create metatable for file handlers
   luaL_register(L, NULL, lfilehmeta); // methods
   lua_pop(L, 1);

   lua_newtable(L);                    // Table[obj] -> Callback-closure
   lua_pushvalue(L, -1); 
   lua_setfield(L, LUA_REGISTRYINDEX, LUA_LUEOBJ);
   lua_pushlightuserdata(L, lueContext);// Table[1] = lueContext
   lua_rawseti(L, -2, 1);
   lua_pop(L, 1);

   return 0;
}


// vim: set sw=3 sts=3 et:
