#include "lualame.h"

#if __GNUC__ > 4
#define LUALAME_PRIVATE __attribute__ ((visibility ("hidden")))
#else
#define LUALAME_PRIVATE
#endif

typedef struct lualame_metamethod_s {
    const char *name;
    const char *metaname;
} lualame_metamethod;

typedef struct lualame_function_up_s {
    const char *name;
    lua_CFunction func;
    void *upfunc;
} lualame_function_up;

#define LUALAME_GET_INT(x) { "lame_get_" #x, lualame_lame_get_int, lame_get_ ## x }
#define LUALAME_SET_INT(x) { "lame_set_" #x, lualame_lame_set_int, lame_set_ ## x }

#define LUALAME_GET_ULONG(x) { "lame_get_" #x, lualame_lame_get_unsigned_long, lame_get_ ## x }
#define LUALAME_SET_ULONG(x) { "lame_set_" #x, lualame_lame_set_unsigned_long, lame_set_ ## x }

#define LUALAME_GET_FLOAT(x) { "lame_get_" #x, lualame_lame_get_float, lame_get_ ## x }
#define LUALAME_SET_FLOAT(x) { "lame_set_" #x, lualame_lame_set_float, lame_set_ ## x }

#define LUALAME_GET_DOUBLE(x) { "lame_get_" #x, lualame_lame_get_double, lame_get_ ## x }
#define LUALAME_SET_DOUBLE(x) { "lame_set_" #x, lualame_lame_set_double, lame_set_ ## x }

#define LUALAME_SET_INT_INT(x) { "lame_set_" #x, lualame_lame_set_int_int, lame_set_ ## x }

#define LUALAME_GET(x) { "lame_get_" #x, "get_" #x }
#define LUALAME_SET(x) { "lame_set_" #x, "set_" #x }

#if (!defined LUA_VERSION_NUM) || LUA_VERSION_NUM == 501
#define lua_setuservalue(L,i) lua_setfenv((L),(i))
#define lua_getuservalue(L,i) lua_getfenv((L),(i))
#define lua_rawlen(L,i) lua_objlen((L),(i))
#endif

#define lualame_push_const(x) lua_pushinteger(L,x) ; lua_setfield(L,-2, #x)

#ifdef __cplusplus
extern "C" {
#endif


#if !defined(luaL_newlibtable) \
  && (!defined LUA_VERSION_NUM || LUA_VERSION_NUM==501)
LUALAME_PRIVATE
void luaL_setfuncs (lua_State *L, const luaL_Reg *l, int nup);

LUALAME_PRIVATE
void luaL_setmetatable(lua_State *L, const char *str);

LUALAME_PRIVATE
void *luaL_testudata (lua_State *L, int i, const char *tname);
#endif

#ifdef __cplusplus
}
#endif
