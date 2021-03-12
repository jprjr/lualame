#include "lualame.h"
#include "lualame_internal.h"
#include <lame/lame.h>

static const char * const lualame_lame_t_mt = "lame_t";

#define FRAME_BUFFER_LEN  2048
#define MP3_BUFFER_LEN    9760 /* 1.25 * frames + 7200 */

#define LUALAME_MIN(a,b) ( (a) < (b) ? (a) : (b) )

struct lualame_lame_s {
    lame_t l;
    double buffer[FRAME_BUFFER_LEN * 2];
    double *buffer_l;
    double *buffer_r;
    unsigned char mp3buf[MP3_BUFFER_LEN];
};

typedef struct lualame_lame_s lualame_lame_t;

typedef int CDECL (*lualame_set_unsigned_long_func)(lame_t, unsigned long);
typedef unsigned long CDECL (*lualame_get_unsigned_long_func)(lame_t);
typedef int CDECL (*lualame_set_int_func)(lame_t, int);
typedef int CDECL (*lualame_get_int_func)(lame_t);
typedef int CDECL (*lualame_set_float_func)(lame_t, float);
typedef float CDECL (*lualame_get_float_func)(lame_t);
typedef int CDECL (*lualame_set_int_int_func)(lame_t, int, int);

typedef void CDECL (*lualame_id3tag_set_void)(lame_t);
typedef void CDECL (*lualame_id3tag_set_str)(lame_t, const char *);
typedef int CDECL (*lualame_id3tag_int_set_str)(lame_t, const char *);
typedef size_t CDECL (*lualame_id3tag_get)(lame_t, unsigned char *, size_t);

typedef int CDECL(*lualame_encode_func)(lame_t, void *l, void *r, const int nsamples, unsigned char *mp3buf, const int mp3buf_size);

typedef int CDECL(*lualame_encode_interleaved_func)(lame_t, void *l, const int nsamples, unsigned char *mp3buf, const int mp3buf_size);

typedef void (*lualame_copyin_func)(lua_State *L, int idx, void *buffer, int nsamples, int offset);

static void
copydown(lua_State *L, const char *tablename) {
    lua_getglobal(L,"require");
    lua_pushstring(L,tablename);
    lua_call(L,1,1);

    /* copies keys from table on top of stack to table below */
    lua_pushnil(L);
    while(lua_next(L,-2) != 0) {
        /* -1 = value
         * -2 = key */
        lua_pushvalue(L,-2);
        lua_insert(L,-2);
        lua_settable(L,-5);
    }
    lua_pop(L,1);
}

static int
lualame_lame_init(lua_State *L) {
    lualame_lame_t *u = lua_newuserdata(L,sizeof(lualame_lame_t));
    if(u == NULL) {
        return luaL_error(L,"out of memory");
    }
    u->l = lame_init();
    if(u->l == NULL) {
        return luaL_error(L,"error allocating lame encoder");
    }
    u->buffer_l = &u->buffer[0];
    u->buffer_r = &u->buffer[FRAME_BUFFER_LEN];
    luaL_setmetatable(L,lualame_lame_t_mt);
    return 1;
}

static int
lualame_lame_close(lua_State *L) {
    lualame_lame_t *u = NULL;

    u = luaL_checkudata(L,1,lualame_lame_t_mt);
    if(u->l != NULL) {
        lame_close(u->l);
        u->l = NULL;
    }
    return 0;
}

static int
lualame_lame_set_unsigned_long(lua_State *L) {
    lualame_lame_t *u = NULL;
    unsigned long val = 0;
    lualame_set_unsigned_long_func f = NULL;

    u = luaL_checkudata(L,1,lualame_lame_t_mt);
    val = luaL_checkinteger(L,2);
    f = lua_touserdata(L,lua_upvalueindex(1));

    lua_pushboolean(L,f(u->l,val) == 0);
    return 1;
}

static int
lualame_lame_get_unsigned_long(lua_State *L) {
    lualame_lame_t *u = NULL;
    lualame_get_unsigned_long_func f = NULL;

    u = luaL_checkudata(L,1,lualame_lame_t_mt);
    f = lua_touserdata(L,lua_upvalueindex(1));

    lua_pushinteger(L,f(u->l));
    return 1;
}

static int
lualame_lame_set_int(lua_State *L) {
    lualame_lame_t *u = NULL;
    int val = 0;
    lualame_set_int_func f = NULL;

    u = luaL_checkudata(L,1,lualame_lame_t_mt);
    val = luaL_checkinteger(L,2);
    f = lua_touserdata(L,lua_upvalueindex(1));

    lua_pushboolean(L,f(u->l,val) == 0);
    return 1;
}

static int
lualame_lame_set_int_void(lua_State *L) {
    lualame_lame_t *u = NULL;
    int val = 0;
    lualame_set_int_func f = NULL;

    u = luaL_checkudata(L,1,lualame_lame_t_mt);
    val = luaL_checkinteger(L,2);
    f = lua_touserdata(L,lua_upvalueindex(1));

    f(u->l,val);
    return 0;
}

static int
lualame_lame_get_int(lua_State *L) {
    lualame_lame_t *u = NULL;
    lualame_get_int_func f = NULL;

    u = luaL_checkudata(L,1,lualame_lame_t_mt);
    f = lua_touserdata(L,lua_upvalueindex(1));

    lua_pushinteger(L,f(u->l));
    return 1;
}

static int
lualame_lame_set_float(lua_State *L) {
    lualame_lame_t *u = NULL;
    float val = 0;
    lualame_set_float_func f = NULL;

    u = luaL_checkudata(L,1,lualame_lame_t_mt);
    val = luaL_checknumber(L,2);
    f = lua_touserdata(L,lua_upvalueindex(1));

    lua_pushboolean(L,f(u->l,val) == 0);
    return 1;
}

static int
lualame_lame_get_float(lua_State *L) {
    lualame_lame_t *u = NULL;
    lualame_get_float_func f = NULL;

    u = luaL_checkudata(L,1,lualame_lame_t_mt);
    f = lua_touserdata(L,lua_upvalueindex(1));

    lua_pushnumber(L,f(u->l));
    return 1;
}

static int
lualame_lame_set_int_int(lua_State *L) {
    lualame_lame_t *u = NULL;
    int val1 = 0;
    int val2 = 0;
    lualame_set_int_int_func f = NULL;

    u = luaL_checkudata(L,1,lualame_lame_t_mt);
    val1 = luaL_checkinteger(L,2);
    val2 = luaL_checkinteger(L,3);
    f = lua_touserdata(L,lua_upvalueindex(1));

    lua_pushboolean(L,f(u->l,val1,val2) == 0);
    return 1;
}

static int
lualame_lame_set_msfix(lua_State *L) {
    lualame_lame_t *u = NULL;
    double val = 0.0f;

    u = luaL_checkudata(L,1,lualame_lame_t_mt);
    val = luaL_checknumber(L,2);
    lame_set_msfix(u->l,val);
    return 0;
}

static int
lualame_lame_get_maximum_number_of_samples(lua_State *L) {
    lualame_lame_t *u = NULL;
    size_t val = 0;

    u = luaL_checkudata(L,1,lualame_lame_t_mt);
    val = luaL_checkinteger(L,2);

    lua_pushinteger(L,lame_get_maximum_number_of_samples(u->l,val));
    return 1;
}

static int
lualame_lame_init_params(lua_State *L) {
    lualame_lame_t *u = NULL;
    u = luaL_checkudata(L,1,lualame_lame_t_mt);
    lua_pushboolean(L,lame_init_params(u->l) != -1);
    return 1;
}

static void
lualame_copyin_short(lua_State *L, int idx, short int *buffer, int nsamples, int offset) {
    int i = nsamples;
    while(i) {
        lua_rawgeti(L,idx,offset + i--);
        buffer[i] = lua_tointeger(L,-1);
        lua_pop(L,1);
    }
}

static void
lualame_copyin_long(lua_State *L, int idx, long *buffer, int nsamples, int offset) {
    int i = nsamples;
    while(i) {
        lua_rawgeti(L,idx,offset + i--);
        buffer[i] = lua_tointeger(L,-1);
        lua_pop(L,1);
    }
}

static void
lualame_copyin_int(lua_State *L, int idx, int *buffer, int nsamples, int offset) {
    int i = nsamples;
    while(i) {
        lua_rawgeti(L,idx,offset + i--);
        buffer[i] = lua_tointeger(L,-1);
        lua_pop(L,1);
    }
}

static void
lualame_copyin_float(lua_State *L, int idx, float *buffer, int nsamples, int offset) {
    int i = nsamples;
    while(i) {
        lua_rawgeti(L,idx,offset + i--);
        buffer[i] = lua_tonumber(L,-1);
        lua_pop(L,1);
    }
}

static void
lualame_copyin_double(lua_State *L, int idx, double *buffer, int nsamples, int offset) {
    int i = nsamples;
    while(i) {
        lua_rawgeti(L,idx,offset + i--);
        buffer[i] = lua_tonumber(L,-1);
        lua_pop(L,1);
    }
}

static int
lualame_lame_encode_buffer(lua_State *L) {
    lualame_lame_t *u = NULL;
    int nsamples = 0;
    int bytes = 0;
    int n = 0;
    int r = 0;
    lualame_encode_func encode = NULL;
    lualame_copyin_func copyin = NULL;
    luaL_Buffer buffer;

    u = luaL_checkudata(L,1,lualame_lame_t_mt);
    if(!lua_istable(L,2)) {
        return luaL_error(L,"missing buffer_l");
    }
    if(!lua_istable(L,3)) {
        return luaL_error(L,"missing buffer_r");
    }

    nsamples = (int)lua_rawlen(L,2);
    if((int)lua_rawlen(L,3) != nsamples) {
        return luaL_error(L,"buffer mismatch");
    }

    encode = lua_touserdata(L,lua_upvalueindex(1));
    copyin = lua_touserdata(L,lua_upvalueindex(2));

    luaL_buffinit(L,&buffer);

    while(r < nsamples) {
        n = LUALAME_MIN(FRAME_BUFFER_LEN, nsamples - r);

        copyin(L,2,u->buffer_l,n,r);
        copyin(L,3,u->buffer_r,n,r);

        bytes = encode(u->l,
          u->buffer_l,
          u->buffer_r,
          n,
          u->mp3buf,
          MP3_BUFFER_LEN);
        if(bytes < 0) {
            lua_pushnil(L);
            lua_pushinteger(L,bytes);
            return 2;
        }
        luaL_addlstring(&buffer,(const char *)u->mp3buf,(size_t)bytes);
        r += n;
    }
    luaL_pushresult(&buffer);
    return 1;
}

static int
lualame_lame_encode_buffer_interleaved(lua_State *L) {
    lualame_lame_t *u = NULL;
    int nsamples = 0;
    int bytes = 0;
    int n = 0;
    int r = 0;
    luaL_Buffer buffer;
    lualame_encode_interleaved_func encode = NULL;
    lualame_copyin_func copyin = NULL;

    u = luaL_checkudata(L,1,lualame_lame_t_mt);
    if(!lua_istable(L,2)) {
        return luaL_error(L,"missing buffer");
    }

    nsamples = (int)lua_rawlen(L,2);
    if(nsamples % 2 != 0) {
        return luaL_error(L,"odd number of samples");
    }

    encode = lua_touserdata(L,lua_upvalueindex(1));
    copyin = lua_touserdata(L,lua_upvalueindex(2));

    luaL_buffinit(L,&buffer);

    while(r < nsamples) {
        n = LUALAME_MIN(FRAME_BUFFER_LEN * 2, nsamples - r);

        copyin(L,2,u->buffer,n, r);

        bytes = encode(u->l,
          u->buffer,
          n/2,
          u->mp3buf,
          MP3_BUFFER_LEN);

        if(bytes < 0) {
            lua_pushnil(L);
            lua_pushinteger(L,bytes);
            return 2;
        }
        luaL_addlstring(&buffer,(const char *)u->mp3buf,(size_t)bytes);
        r += n;
    }
    luaL_pushresult(&buffer);
    return 1;
}

static int
lualame_lame_encode_flush(lua_State *L) {
    lualame_lame_t *u = NULL;
    int bytes = 0;

    u = luaL_checkudata(L,1,lualame_lame_t_mt);

    bytes = lame_encode_flush(u->l,
      u->mp3buf,
      MP3_BUFFER_LEN);

    if(bytes < 0) {
        lua_pushnil(L);
        lua_pushinteger(L,bytes);
        return 2;
    }
    lua_pushlstring(L,(const char *)u->mp3buf,bytes);
    return 1;
}

static int
lualame_lame_encode_flush_nogap(lua_State *L) {
    lualame_lame_t *u = NULL;
    int bytes = 0;

    u = luaL_checkudata(L,1,lualame_lame_t_mt);

    bytes = lame_encode_flush_nogap(u->l,
      u->mp3buf,
      MP3_BUFFER_LEN);

    if(bytes < 0) {
        lua_pushnil(L);
        lua_pushinteger(L,bytes);
        return 2;
    }
    lua_pushlstring(L,(const char *)u->mp3buf,bytes);
    return 1;
}

struct lualame__id3tag_state_s {
    lua_State *L;
    int index;
};

typedef struct lualame__id3tag_state_s lualame__id3tag_state;

static void
lualame__id3tag_genre_handler(int id, const char *name, void *cookie) {
    lualame__id3tag_state *state = cookie;
    lua_newtable(state->L);
    lua_pushstring(state->L,name);
    lua_setfield(state->L,-2,"name");
    lua_pushinteger(state->L,id);
    lua_setfield(state->L,-2,"id");
    lua_rawseti(state->L,-2,state->index++);
}


static int
lualame_id3tag_genre_list(lua_State *L) {
    lualame__id3tag_state state;
    state.L = L;
    state.index = 1;
    lua_newtable(L);
    id3tag_genre_list(lualame__id3tag_genre_handler,&state);
    return 1;
}

static int
lualame_id3tag_void(lua_State *L) {
    lualame_lame_t *u = NULL;
    lualame_id3tag_set_void f = NULL;

    u = luaL_checkudata(L,1,lualame_lame_t_mt);
    f = lua_touserdata(L,lua_upvalueindex(1));

    f(u->l);
    return 0;
}

static int
lualame_id3tag_set_pad(lua_State *L) {
    lualame_lame_t *u = NULL;
    size_t n = 0;

    u = luaL_checkudata(L,1,lualame_lame_t_mt);
    n = luaL_checkinteger(L,2);

    id3tag_set_pad(u->l,n);
    return 0;
}

static int
lualame_id3tag_str(lua_State *L) {
    lualame_lame_t *u = NULL;
    const char *val = NULL;
    lualame_id3tag_set_str f = NULL;

    u = luaL_checkudata(L,1,lualame_lame_t_mt);
    val = luaL_checkstring(L,2);
    f = lua_touserdata(L,lua_upvalueindex(1));

    f(u->l,val);
    return 0;
}

static int
lualame_id3tag_int_str(lua_State *L) {
    lualame_lame_t *u = NULL;
    const char *val = NULL;
    lualame_id3tag_int_set_str f = NULL;

    u = luaL_checkudata(L,1,lualame_lame_t_mt);
    val = luaL_checkstring(L,2);
    f = lua_touserdata(L,lua_upvalueindex(1));

    lua_pushboolean(L,f(u->l,val) == 0);
    return 1;
}

static int
lualame_id3tag_set_albumart(lua_State *L) {
    lualame_lame_t *u = NULL;
    const char *image = NULL;
    size_t size = 0;

    u = luaL_checkudata(L,1,lualame_lame_t_mt);
    image = luaL_checklstring(L,2,&size);

    lua_pushboolean(L,id3tag_set_albumart(u->l,image,size));
    return 1;
}

static int
lualame_get_id3_tag(lua_State *L) {
    lualame_lame_t *u = NULL;
    unsigned char *buffer = 0;
    size_t len = 0;
    size_t r = 0;
    lualame_id3tag_get f = NULL;

    u = luaL_checkudata(L,1,lualame_lame_t_mt);
    f = lua_touserdata(L,lua_upvalueindex(1));

    r = f(u->l,NULL,0);
    if(r) {
        len = r;
        buffer = lua_newuserdata(L,len);
        if(buffer == NULL) {
            return luaL_error(L,"out of memory");
        }
        r = f(u->l, buffer, len);
        if(r != len) {
            return luaL_error(L,"error copying tag");
        }
        lua_pushlstring(L,(const char *)buffer,len);
        return 1;
    }

    return 0;
}

static const struct luaL_Reg lualame_functions[] = {
    { "lame_init", lualame_lame_init },
    { "lame_close", lualame_lame_close },
    { "lame_set_msfix", lualame_lame_set_msfix },
    { "lame_get_maximum_number_of_samples", lualame_lame_get_maximum_number_of_samples },
    { "lame_init_params", lualame_lame_init_params },
    { "lame_encode_flush", lualame_lame_encode_flush },
    { "lame_encode_flush_nogap", lualame_lame_encode_flush_nogap },
    { "id3tag_genre_list", lualame_id3tag_genre_list },
    { "id3tag_set_pad", lualame_id3tag_set_pad },
    { "id3tag_set_albumart", lualame_id3tag_set_albumart },
    { NULL, NULL },
};

static const lualame_function_up lualame_ups[] = {
    LUALAME_GET_ULONG(num_samples),
    LUALAME_SET_ULONG(num_samples),
    LUALAME_GET_INT(in_samplerate),
    LUALAME_SET_INT(in_samplerate),
    LUALAME_GET_INT(num_channels),
    LUALAME_SET_INT(num_channels),
    LUALAME_GET_FLOAT(scale),
    LUALAME_SET_FLOAT(scale),
    LUALAME_GET_FLOAT(scale_left),
    LUALAME_SET_FLOAT(scale_left),
    LUALAME_GET_FLOAT(scale_right),
    LUALAME_SET_FLOAT(scale_right),
    LUALAME_GET_INT(out_samplerate),
    LUALAME_SET_INT(out_samplerate),
    LUALAME_GET_INT(analysis),
    LUALAME_SET_INT(analysis),
    LUALAME_GET_INT(bWriteVbrTag),
    LUALAME_SET_INT(bWriteVbrTag),
    LUALAME_GET_INT(decode_only),
    LUALAME_SET_INT(decode_only),
#ifndef DEPRECATED_OR_OBSOLETE_CODE_REMOVED
    LUALAME_GET_INT(ogg),
    LUALAME_SET_INT(ogg),
#endif
    LUALAME_GET_INT(quality),
    LUALAME_SET_INT(quality),
    LUALAME_GET_INT(mode),
    LUALAME_SET_INT(mode),
#ifndef DEPRECATED_OR_OBSOLETE_CODE_REMOVED
    LUALAME_GET_INT(mode_automs),
    LUALAME_SET_INT(mode_automs),
#endif
    LUALAME_GET_INT(force_ms),
    LUALAME_SET_INT(force_ms),
    LUALAME_GET_INT(free_format),
    LUALAME_SET_INT(free_format),
    LUALAME_GET_INT(findReplayGain),
    LUALAME_SET_INT(findReplayGain),
    LUALAME_GET_INT(decode_on_the_fly),
    LUALAME_SET_INT(decode_on_the_fly),
#ifndef DEPRECATED_OR_OBSOLETE_CODE_REMOVED
    LUALAME_GET_INT(ReplayGain_input),
    LUALAME_SET_INT(ReplayGain_input),
    LUALAME_GET_INT(ReplayGain_decode),
    LUALAME_SET_INT(ReplayGain_decode),
    LUALAME_GET_INT(findPeakSample),
    LUALAME_SET_INT(findPeakSample),
#endif
    LUALAME_GET_INT(nogap_total),
    LUALAME_SET_INT(nogap_total),
    LUALAME_GET_INT(nogap_currentindex),
    LUALAME_SET_INT(nogap_currentindex),
    LUALAME_GET_INT(brate),
    LUALAME_SET_INT(brate),
    LUALAME_GET_FLOAT(compression_ratio),
    LUALAME_SET_FLOAT(compression_ratio),
    LUALAME_SET_INT(preset),
    LUALAME_SET_INT_INT(asm_optimizations),
    LUALAME_GET_INT(copyright),
    LUALAME_SET_INT(copyright),
    LUALAME_GET_INT(original),
    LUALAME_SET_INT(original),
    LUALAME_GET_INT(error_protection),
    LUALAME_SET_INT(error_protection),
#ifndef DEPRECATED_OR_OBSOLETE_CODE_REMOVED
    LUALAME_GET_INT(padding_type),
    LUALAME_SET_INT(padding_type),
#endif
    LUALAME_GET_INT(extension),
    LUALAME_SET_INT(extension),
    LUALAME_GET_INT(strict_ISO),
    LUALAME_SET_INT(strict_ISO),
    LUALAME_GET_INT(disable_reservoir),
    LUALAME_SET_INT(disable_reservoir),
    LUALAME_GET_INT(quant_comp),
    LUALAME_SET_INT(quant_comp),
    LUALAME_GET_INT(quant_comp_short),
    LUALAME_SET_INT(quant_comp_short),
    LUALAME_GET_INT(experimentalX),
    LUALAME_SET_INT(experimentalX),
    LUALAME_GET_INT(experimentalY),
    LUALAME_SET_INT(experimentalY),
    LUALAME_GET_INT(experimentalZ),
    LUALAME_SET_INT(experimentalZ),
    LUALAME_GET_INT(exp_nspsytune),
    LUALAME_SET_INT(exp_nspsytune),
    /* set_msfix covered as regular function */
    LUALAME_GET_FLOAT(msfix),
    LUALAME_GET_INT(VBR),
    LUALAME_SET_INT(VBR),
    LUALAME_GET_INT(VBR_q),
    LUALAME_SET_INT(VBR_q),
    LUALAME_GET_FLOAT(VBR_quality),
    LUALAME_SET_FLOAT(VBR_quality),
    LUALAME_GET_INT(VBR_mean_bitrate_kbps),
    LUALAME_SET_INT(VBR_mean_bitrate_kbps),
    LUALAME_GET_INT(VBR_min_bitrate_kbps),
    LUALAME_SET_INT(VBR_min_bitrate_kbps),
    LUALAME_GET_INT(VBR_max_bitrate_kbps),
    LUALAME_SET_INT(VBR_max_bitrate_kbps),
    LUALAME_GET_INT(VBR_hard_min),
    LUALAME_SET_INT(VBR_hard_min),
#ifndef DEPRECATED_OR_OBSOLETE_CODE_REMOVED
    LUALAME_SET_INT(preset_expopts),
#endif
    LUALAME_GET_INT(lowpassfreq),
    LUALAME_SET_INT(lowpassfreq),
    LUALAME_GET_INT(lowpasswidth),
    LUALAME_SET_INT(lowpasswidth),
    LUALAME_GET_INT(highpassfreq),
    LUALAME_SET_INT(highpassfreq),
    LUALAME_GET_INT(highpasswidth),
    LUALAME_SET_INT(highpasswidth),
    LUALAME_GET_INT(ATHonly),
    LUALAME_SET_INT(ATHonly),
    LUALAME_GET_INT(ATHshort),
    LUALAME_SET_INT(ATHshort),
    LUALAME_GET_INT(noATH),
    LUALAME_SET_INT(noATH),
    LUALAME_GET_INT(ATHtype),
    LUALAME_SET_INT(ATHtype),
    LUALAME_GET_FLOAT(ATHlower),
    LUALAME_SET_FLOAT(ATHlower),
    LUALAME_GET_INT(athaa_type),
    LUALAME_SET_INT(athaa_type),
#ifndef DEPRECATED_OR_OBSOLETE_CODE_REMOVED
    LUALAME_GET_INT(athaa_loudapprox),
    LUALAME_SET_INT(athaa_loudapprox),
#endif
    LUALAME_GET_FLOAT(athaa_sensitivity),
    LUALAME_SET_FLOAT(athaa_sensitivity),
#ifndef DEPRECATED_OR_OBSOLETE_CODE_REMOVED
    LUALAME_GET_INT(cwlimit),
    LUALAME_SET_INT(cwlimit),
#endif
    LUALAME_GET_INT(allow_diff_short),
    LUALAME_SET_INT(allow_diff_short),
    LUALAME_GET_INT(useTemporal),
    LUALAME_SET_INT(useTemporal),
    LUALAME_GET_INT(interChRatio),
    LUALAME_SET_INT(interChRatio),
    LUALAME_GET_INT(no_short_blocks),
    LUALAME_SET_INT(no_short_blocks),
    LUALAME_GET_INT(force_short_blocks),
    LUALAME_SET_INT(force_short_blocks),
    LUALAME_GET_INT(emphasis),
    LUALAME_SET_INT(emphasis),
    LUALAME_GET_INT(version),
    LUALAME_GET_INT(encoder_delay),
    LUALAME_GET_INT(encoder_padding),
    LUALAME_GET_INT(framesize),
    LUALAME_GET_INT(mf_samples_to_encode),
    LUALAME_GET_INT(size_mp3buffer),
    LUALAME_GET_INT(frameNum),
    LUALAME_GET_INT(totalframes),
    LUALAME_GET_INT(RadioGain),
    LUALAME_GET_INT(AudiophileGain),
    LUALAME_GET_FLOAT(PeakSample),
    LUALAME_GET_INT(noclipGainChange),
    LUALAME_GET_FLOAT(noclipScale),
    { "id3tag_init", lualame_id3tag_void, id3tag_init },
    { "id3tag_add_v2", lualame_id3tag_void, id3tag_add_v2 },
    { "id3tag_v1_only", lualame_id3tag_void, id3tag_v1_only },
    { "id3tag_v2_only", lualame_id3tag_void, id3tag_v2_only },
    { "id3tag_space_v1", lualame_id3tag_void, id3tag_space_v1 },
    { "id3tag_pad_v2", lualame_id3tag_void, id3tag_pad_v2 },
    { "id3tag_set_title", lualame_id3tag_str, id3tag_set_title },
    { "id3tag_set_artist", lualame_id3tag_str, id3tag_set_artist },
    { "id3tag_set_album", lualame_id3tag_str, id3tag_set_album },
    { "id3tag_set_year", lualame_id3tag_str, id3tag_set_year },
    { "id3tag_set_comment", lualame_id3tag_str, id3tag_set_comment },
    { "id3tag_set_track", lualame_id3tag_int_str, id3tag_set_track },
    { "id3tag_set_genre", lualame_id3tag_int_str, id3tag_set_genre },
    { "id3tag_set_fieldvalue", lualame_id3tag_int_str, id3tag_set_fieldvalue },
    { "lame_get_id3v1_tag", lualame_get_id3_tag, lame_get_id3v1_tag },
    { "lame_get_id3v2_tag", lualame_get_id3_tag, lame_get_id3v2_tag },
    { "lame_set_write_id3tag_automatic",lualame_lame_set_int_void , lame_set_write_id3tag_automatic },
    LUALAME_GET_INT(write_id3tag_automatic),
    { NULL, NULL, NULL },
};

static const lualame_metamethod lualame_lame_metamethods[] = {
    { "lame_close", "close" },
    { "lame_init_params", "init_params" },
    { "lame_encode_buffer", "encode_buffer" },
    { "lame_encode_buffer_interleaved", "encode_buffer_interleaved" },
    { "lame_encode_buffer_float", "encode_buffer_float" },
    { "lame_encode_buffer_ieee_float", "encode_buffer_ieee_float" },
    { "lame_encode_buffer_interleaved_ieee_float", "encode_buffer_interleaved_ieee_float" },
    { "lame_encode_buffer_ieee_double", "encode_buffer_ieee_double" },
    { "lame_encode_buffer_interleaved_ieee_double", "encode_buffer_interleaved_ieee_double" },
    { "lame_encode_buffer_long", "encode_buffer_long" },
    { "lame_encode_buffer_long2", "encode_buffer_long2" },
    { "lame_encode_buffer_int", "encode_buffer_int" },
    { "lame_encode_buffer_interleaved_int", "encode_buffer_interleaved_int" },
    { "lame_encode_flush", "encode_flush" },
    { "lame_encode_flush_nogap", "encode_flush_nogap" },
    { "id3tag_genre_list", "id3tag_genre_list" },
    { "id3tag_init", "id3tag_init" },
    { "id3tag_add_v2", "id3tag_add_v2" },
    { "id3tag_v1_only", "id3tag_v1_only" },
    { "id3tag_v2_only", "id3tag_v2_only" },
    { "id3tag_space_v1", "id3tag_space_v1" },
    { "id3tag_pad_v2", "id3tag_pad_v2" },
    { "id3tag_set_pad", "id3tag_set_pad" },
    { "id3tag_set_title", "id3tag_set_title" },
    { "id3tag_set_artist", "id3tag_set_artist" },
    { "id3tag_set_album", "id3tag_set_album" },
    { "id3tag_set_year", "id3tag_set_year" },
    { "id3tag_set_comment", "id3tag_set_comment" },
    { "id3tag_set_track", "id3tag_set_track" },
    { "id3tag_set_genre", "id3tag_set_genre" },
    { "id3tag_set_fieldvalue", "id3tag_set_fieldvalue" },
    { "id3tag_set_albumart", "id3tag_set_albumart" },
    { "lame_get_id3v1_tag", "get_id3v1_tag" },
    { "lame_get_id3v2_tag", "get_id3v2_tag" },
    LUALAME_GET(write_id3tag_automatic), LUALAME_SET(write_id3tag_automatic),
    LUALAME_GET(num_samples), LUALAME_SET(num_samples),
    LUALAME_GET(num_channels), LUALAME_SET(num_channels),
    LUALAME_GET(in_samplerate), LUALAME_SET(in_samplerate),
    LUALAME_GET(scale), LUALAME_SET(scale),
    LUALAME_GET(scale_left), LUALAME_SET(scale_left),
    LUALAME_GET(scale_right), LUALAME_SET(scale_right),
    LUALAME_GET(out_samplerate), LUALAME_SET(out_samplerate),
    LUALAME_GET(analysis), LUALAME_SET(analysis),
    LUALAME_GET(bWriteVbrTag), LUALAME_SET(bWriteVbrTag),
    LUALAME_GET(decode_only), LUALAME_SET(decode_only),
#ifndef DEPRECATED_OR_OBSOLETE_CODE_REMOVED
    LUALAME_GET(ogg), LUALAME_SET(ogg),
#endif
    LUALAME_GET(quality), LUALAME_SET(quality),
    LUALAME_GET(mode), LUALAME_SET(mode),
#ifndef DEPRECATED_OR_OBSOLETE_CODE_REMOVED
    LUALAME_GET(mode_automs), LUALAME_SET(mode_automs),
#endif
    LUALAME_GET(force_ms), LUALAME_SET(force_ms),
    LUALAME_GET(free_format), LUALAME_SET(free_format),
    LUALAME_GET(findReplayGain), LUALAME_SET(findReplayGain),
    LUALAME_GET(decode_on_the_fly), LUALAME_SET(decode_on_the_fly),
#ifndef DEPRECATED_OR_OBSOLETE_CODE_REMOVED
    LUALAME_GET(ReplayGain_input), LUALAME_SET(ReplayGain_input),
    LUALAME_GET(ReplayGain_decode), LUALAME_SET(ReplayGain_decode),
    LUALAME_GET(findPeakSample), LUALAME_SET(findPeakSample),
#endif
    LUALAME_GET(nogap_total), LUALAME_SET(nogap_total),
    LUALAME_GET(nogap_currentindex), LUALAME_SET(nogap_currentindex),
    LUALAME_GET(brate), LUALAME_SET(brate),
    LUALAME_GET(compression_ratio), LUALAME_SET(compression_ratio),
    LUALAME_SET(preset), LUALAME_SET(asm_optimizations),
    LUALAME_GET(copyright), LUALAME_SET(copyright),
    LUALAME_GET(original), LUALAME_SET(original),
    LUALAME_GET(error_protection), LUALAME_SET(error_protection),
#ifndef DEPRECATED_OR_OBSOLETE_CODE_REMOVED
    LUALAME_GET(padding_type), LUALAME_SET(padding_type),
#endif
    LUALAME_GET(extension), LUALAME_SET(extension),
    LUALAME_GET(strict_ISO), LUALAME_SET(strict_ISO),
    LUALAME_GET(disable_reservoir), LUALAME_SET(disable_reservoir),
    LUALAME_GET(quant_comp), LUALAME_SET(quant_comp),
    LUALAME_GET(quant_comp_short), LUALAME_SET(quant_comp_short),
    LUALAME_GET(experimentalX), LUALAME_SET(experimentalX),
    LUALAME_GET(experimentalY), LUALAME_SET(experimentalY),
    LUALAME_GET(experimentalZ), LUALAME_SET(experimentalZ),
    LUALAME_GET(exp_nspsytune), LUALAME_SET(exp_nspsytune),
    LUALAME_SET(msfix), LUALAME_GET(msfix),
    LUALAME_SET(VBR), LUALAME_GET(VBR),
    LUALAME_GET(VBR_q), LUALAME_SET(VBR_q),
    LUALAME_GET(VBR_quality), LUALAME_SET(VBR_quality),
    LUALAME_GET(VBR_mean_bitrate_kbps), LUALAME_SET(VBR_mean_bitrate_kbps),
    LUALAME_GET(VBR_min_bitrate_kbps), LUALAME_SET(VBR_min_bitrate_kbps),
    LUALAME_GET(VBR_max_bitrate_kbps), LUALAME_SET(VBR_max_bitrate_kbps),
    LUALAME_GET(VBR_hard_min), LUALAME_SET(VBR_hard_min),
#ifndef DEPRECATED_OR_OBSOLETE_CODE_REMOVED
    LUALAME_SET(preset_expopts),
#endif
    LUALAME_GET(lowpassfreq), LUALAME_SET(lowpassfreq),
    LUALAME_GET(lowpasswidth), LUALAME_SET(lowpasswidth),
    LUALAME_GET(highpassfreq), LUALAME_SET(highpassfreq),
    LUALAME_GET(highpasswidth), LUALAME_SET(highpasswidth),
    LUALAME_GET(ATHonly), LUALAME_SET(ATHonly),
    LUALAME_GET(ATHshort), LUALAME_SET(ATHshort),
    LUALAME_GET(noATH), LUALAME_SET(noATH),
    LUALAME_GET(ATHtype), LUALAME_SET(ATHtype),
    LUALAME_GET(ATHlower), LUALAME_SET(ATHlower),
    LUALAME_GET(athaa_type),
    LUALAME_SET(athaa_type),
#ifndef DEPRECATED_OR_OBSOLETE_CODE_REMOVED
    LUALAME_GET(athaa_loadapproax),
    LUALAME_SET(athaa_loadapproax),
#endif
    LUALAME_GET(athaa_sensitivity),
    LUALAME_SET(athaa_sensitivity),
#ifndef DEPRECATED_OR_OBSOLETE_CODE_REMOVED
    LUALAME_GET(cwlimit),
    LUALAME_SET(cwlimit),
#endif
    LUALAME_GET(allow_diff_short),
    LUALAME_SET(allow_diff_short),
    LUALAME_GET(useTemporal),
    LUALAME_SET(useTemporal),
    LUALAME_GET(interChRatio),
    LUALAME_SET(interChRatio),
    LUALAME_GET(no_short_blocks),
    LUALAME_SET(no_short_blocks),
    LUALAME_GET(force_short_blocks),
    LUALAME_SET(force_short_blocks),
    LUALAME_GET(emphasis),
    LUALAME_SET(emphasis),
    LUALAME_GET(version),
    LUALAME_GET(encoder_delay),
    LUALAME_GET(encoder_padding),
    LUALAME_GET(framesize),
    LUALAME_GET(mf_samples_to_encode),
    LUALAME_GET(size_mp3buffer),
    LUALAME_GET(frameNum),
    LUALAME_GET(totalframes),
    LUALAME_GET(RadioGain),
    LUALAME_GET(AudiophileGain),
    LUALAME_GET(PeakSample),
    LUALAME_GET(noclipGainChange),
    LUALAME_GET(noclipScale),
    LUALAME_GET(maximum_number_of_samples),
    { NULL, NULL },
};

LUALAME_PUBLIC
int luaopen_lualame(lua_State *L) {
    const lualame_metamethod *lame_mm = lualame_lame_metamethods;
    const lualame_function_up *ups = lualame_ups;
    lua_newtable(L);

    copydown(L,"lualame.version");
    copydown(L,"lualame.defines");

    luaL_setfuncs(L,lualame_functions,0);

    lua_pushlightuserdata(L,lame_encode_buffer);
    lua_pushlightuserdata(L,lualame_copyin_short);
    lua_pushcclosure(L,lualame_lame_encode_buffer,2);
    lua_setfield(L,-2,"lame_encode_buffer");

    lua_pushlightuserdata(L,lame_encode_buffer_interleaved);
    lua_pushlightuserdata(L,lualame_copyin_short);
    lua_pushcclosure(L,lualame_lame_encode_buffer_interleaved,2);
    lua_setfield(L,-2,"lame_encode_buffer_interleaved");

    lua_pushlightuserdata(L,lame_encode_buffer_float);
    lua_pushlightuserdata(L,lualame_copyin_float);
    lua_pushcclosure(L,lualame_lame_encode_buffer,2);
    lua_setfield(L,-2,"lame_encode_buffer_float");

    lua_pushlightuserdata(L,lame_encode_buffer_ieee_float);
    lua_pushlightuserdata(L,lualame_copyin_float);
    lua_pushcclosure(L,lualame_lame_encode_buffer,2);
    lua_setfield(L,-2,"lame_encode_buffer_ieee_float");

    lua_pushlightuserdata(L,lame_encode_buffer_interleaved_ieee_float);
    lua_pushlightuserdata(L,lualame_copyin_float);
    lua_pushcclosure(L,lualame_lame_encode_buffer_interleaved,2);
    lua_setfield(L,-2,"lame_encode_buffer_interleaved_ieee_float");

    lua_pushlightuserdata(L,lame_encode_buffer_ieee_double);
    lua_pushlightuserdata(L,lualame_copyin_double);
    lua_pushcclosure(L,lualame_lame_encode_buffer,2);
    lua_setfield(L,-2,"lame_encode_buffer_ieee_double");

    lua_pushlightuserdata(L,lame_encode_buffer_interleaved_ieee_double);
    lua_pushlightuserdata(L,lualame_copyin_double);
    lua_pushcclosure(L,lualame_lame_encode_buffer_interleaved,2);
    lua_setfield(L,-2,"lame_encode_buffer_interleaved_ieee_double");

    lua_pushlightuserdata(L,lame_encode_buffer_long);
    lua_pushlightuserdata(L,lualame_copyin_long);
    lua_pushcclosure(L,lualame_lame_encode_buffer,2);
    lua_setfield(L,-2,"lame_encode_buffer_long");

    lua_pushlightuserdata(L,lame_encode_buffer_long2);
    lua_pushlightuserdata(L,lualame_copyin_long);
    lua_pushcclosure(L,lualame_lame_encode_buffer,2);
    lua_setfield(L,-2,"lame_encode_buffer_long2");

    lua_pushlightuserdata(L,lame_encode_buffer_int);
    lua_pushlightuserdata(L,lualame_copyin_int);
    lua_pushcclosure(L,lualame_lame_encode_buffer,2);
    lua_setfield(L,-2,"lame_encode_buffer_int");

    lua_pushlightuserdata(L,lame_encode_buffer_interleaved_int);
    lua_pushlightuserdata(L,lualame_copyin_int);
    lua_pushcclosure(L,lualame_lame_encode_buffer_interleaved,2);
    lua_setfield(L,-2,"lame_encode_buffer_interleaved_int");

    while(ups->name != NULL) {
        lua_pushlightuserdata(L,ups->upfunc);
        lua_pushcclosure(L,ups->func,1);
        lua_setfield(L,-2,ups->name);
        ups++;
    }

    luaL_newmetatable(L,lualame_lame_t_mt);
    lua_getfield(L,-2,"lame_close");
    lua_setfield(L,-2,"__gc");

    lua_newtable(L);
    while(lame_mm->name != NULL) {
        lua_getfield(L,-3,lame_mm->name);
        lua_setfield(L,-2,lame_mm->metaname);
        lame_mm++;
    }
    lua_setfield(L,-2,"__index");
    lua_pop(L,1);

    return 1;
}
