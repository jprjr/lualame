#include <lua.h>
#include <lauxlib.h>

#if defined(_WIN32) || defined(_WIN64) || defined(WIN32) || defined(_MSC_VER)
#define LUALAME_PUBLIC __declspec(dllexport)
#elif __GNUC__ > 4
#define LUALAME_PUBLIC __attribute__ ((visibility ("default")))
#else
#define LUALAME_PUBLIC
#endif

#ifdef __cplusplus
extern "C" {
#endif

LUALAME_PUBLIC
int luaopen_luaopus(lua_State *L);

LUALAME_PUBLIC
int luaopen_luaopus_defines(lua_State *L);

#ifdef __cplusplus
}
#endif
