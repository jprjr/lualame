#include "lualame_internal.h"
#include <lame/lame.h>

static int
lualame_get_lame_version(lua_State *L) {
    lua_pushstring(L,get_lame_version());
    return 1;
}

static int
lualame_get_lame_short_version(lua_State *L) {
    lua_pushstring(L,get_lame_short_version());
    return 1;
}

static int
lualame_get_lame_very_short_version(lua_State *L) {
    lua_pushstring(L,get_lame_very_short_version());
    return 1;
}

static int
lualame_get_psy_version(lua_State *L) {
    lua_pushstring(L,get_psy_version());
    return 1;
}

static int
lualame_get_lame_url(lua_State *L) {
    lua_pushstring(L,get_lame_url());
    return 1;
}

static int
lualame_get_lame_os_bitness(lua_State *L) {
    lua_pushstring(L,get_lame_os_bitness());
    return 1;
}

static int
lualame_get_lame_version_numerical(lua_State *L) {
    lame_version_t t;
    get_lame_version_numerical(&t);
    lua_newtable(L);
    lua_pushinteger(L,t.major);
    lua_setfield(L,-2,"major");
    lua_pushinteger(L,t.minor);
    lua_setfield(L,-2,"minor");
    lua_pushboolean(L,t.alpha);
    lua_setfield(L,-2,"alpha");
    lua_pushboolean(L,t.beta);
    lua_setfield(L,-2,"beta");

    lua_pushinteger(L,t.psy_major);
    lua_setfield(L,-2,"psy_major");
    lua_pushinteger(L,t.psy_minor);
    lua_setfield(L,-2,"psy_minor");
    lua_pushboolean(L,t.psy_alpha);
    lua_setfield(L,-2,"psy_alpha");
    lua_pushboolean(L,t.psy_beta);
    lua_setfield(L,-2,"psy_beta");
    lua_pushstring(L,t.features);
    lua_setfield(L,-2,"features");

    return 1;
}


static const struct luaL_Reg lualame_version_funcs[] = {
    { "get_lame_version", lualame_get_lame_version },
    { "get_lame_short_version", lualame_get_lame_short_version },
    { "get_lame_very_short_version", lualame_get_lame_very_short_version },
    { "get_psy_version", lualame_get_psy_version },
    { "get_lame_url", lualame_get_lame_url },
    { "get_lame_os_bitness", lualame_get_lame_os_bitness },
    { "get_lame_version_numerical", lualame_get_lame_version_numerical },
    { NULL, NULL },
};

LUALAME_PUBLIC
int luaopen_lualame_defines(lua_State *L) {
    lua_newtable(L);

    lualame_push_const(LAME_OKAY);
    lualame_push_const(LAME_NOERROR);
    lualame_push_const(LAME_GENERICERROR);
    lualame_push_const(LAME_NOMEM);
    lualame_push_const(LAME_BADBITRATE);
    lualame_push_const(LAME_BADSAMPFREQ);
    lualame_push_const(LAME_INTERNALERROR);
    lualame_push_const(FRONTEND_READERROR);
    lualame_push_const(FRONTEND_WRITEERROR);
    lualame_push_const(FRONTEND_FILETOOLARGE);
    lualame_push_const(vbr_off);
    lualame_push_const(vbr_mt);
    lualame_push_const(vbr_rh);
    lualame_push_const(vbr_abr);
    lualame_push_const(vbr_mtrh);
    lualame_push_const(vbr_max_indicator);
    lualame_push_const(vbr_default);
    lualame_push_const(STEREO);
    lualame_push_const(JOINT_STEREO);
    lualame_push_const(DUAL_CHANNEL);
    lualame_push_const(MONO);
    lualame_push_const(NOT_SET);
    lualame_push_const(MAX_INDICATOR);
    lualame_push_const(PAD_NO);
    lualame_push_const(PAD_ALL);
    lualame_push_const(PAD_ADJUST);
    lualame_push_const(PAD_MAX_INDICATOR);
    lualame_push_const(ABR_8);
    lualame_push_const(ABR_320);
    lualame_push_const(VBR_10);
    lualame_push_const(VBR_20);
    lualame_push_const(VBR_30);
    lualame_push_const(VBR_40);
    lualame_push_const(VBR_50);
    lualame_push_const(VBR_60);
    lualame_push_const(VBR_70);
    lualame_push_const(VBR_80);
    lualame_push_const(VBR_90);
    lualame_push_const(VBR_100);
    lualame_push_const(V9);
    lualame_push_const(V8);
    lualame_push_const(V7);
    lualame_push_const(V6);
    lualame_push_const(V5);
    lualame_push_const(V4);
    lualame_push_const(V3);
    lualame_push_const(V2);
    lualame_push_const(V1);
    lualame_push_const(V0);
    lualame_push_const(R3MIX);
    lualame_push_const(STANDARD);
    lualame_push_const(EXTREME);
    lualame_push_const(INSANE);
    lualame_push_const(STANDARD_FAST);
    lualame_push_const(EXTREME_FAST);
    lualame_push_const(MEDIUM);
    lualame_push_const(MEDIUM_FAST);
    lualame_push_const(MMX);
    lualame_push_const(AMD_3DNOW);
    lualame_push_const(SSE);
    lualame_push_const(PSY_GPSYCHO);
    lualame_push_const(PSY_NSPSYTUNE);
    lualame_push_const(MDB_DEFAULT);
    lualame_push_const(MDB_STRICT_ISO);
    lualame_push_const(MDB_MAXIMUM);

    luaL_setfuncs(L,lualame_version_funcs,0);

    return 1;
}
