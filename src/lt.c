#include "lt.h"

static const struct luaL_Reg ltracer_lib_f [] = {
    {"new", lt_tracer_new},
    {NULL, NULL}
};

static const struct luaL_Reg ltracer_lib_m [] = {
    {"__gc", lt_tracer_gc},
    {"step", lt_tracer_step},
    {"term", lt_tracer_term},
    {"ip",   lt_tracer_ip},
    {"pid",  lt_tracer_pid},
    {"mem",  lt_tracer_mem},
    {NULL,   NULL}
};

LUALIB_API int luaopen_ltracer (lua_State * L)
{   
    luaL_newmetatable(L, "ltracer.tracer_t");
    lua_pushstring(L, "__index");
    lua_pushvalue(L, -2);
    lua_settable(L, -3);
    luaL_register(L, NULL, ltracer_lib_m);
    luaL_register(L, "tracer_t", ltracer_lib_f);

    return 1;
}


struct _tracer * lt_tracer_check_t (lua_State * L, int position)
{
    struct _tracer * tracer;
    void * userdata = luaL_checkudata(L, position, "ltracer.tracer_t");
    luaL_argcheck(L, userdata != NULL, position, "ltracer.tracer_t expected");
    ud_obj = (struct _tracer *) userdata;
    return tracer;
}


int lt_tracer_new (lua_State * L)
{
    ud_t * ud_obj;
    
    ud_obj = lua_newuserdata(L, sizeof(ud_t));
    luaL_getmetatable(L, "ludis86.ud_t");
    lua_setmetatable(L, -2);
    
    ud_init(ud_obj);
    
    return 1;
}


int ludis86_ud_set_input_buffer (lua_State * L)
{
    ud_t * ud_obj;
    const char * input_buffer;
    size_t buffer_size;
    
    ud_obj       = ludis86_check_ud_t(L, 1);
    input_buffer = lua_tolstring(L, 2, &buffer_size);
    
    lua_pop(L, 2);
    
    ud_set_input_buffer(ud_obj, (uint8_t *) input_buffer, buffer_size);
    
    return 0;
}


int ludis86_ud_set_mode (lua_State * L)
{
    ud_t * ud_obj;
    int mode;
    
    ud_obj = ludis86_check_ud_t(L, 1);
    mode   = luaL_checkint(L, 2);
    
    lua_pop(L, 2);
    
    if (    (mode != 16)
         && (mode != 32)
         && (mode != 64))
        luaL_error(L, "valid modes are 16, 32 and 64");
    
    ud_set_mode(ud_obj, mode);
    
    return 0;
}


int ludis86_ud_set_pc (lua_State * L)
{
    ud_t * ud_obj;
    uint64_t rip;
    
    ud_obj = ludis86_check_ud_t(L, 1);
    
    if (lua_isnumber(L, 2))
        rip = luaL_checkint(L, 2);
    else
        rip = strtoull(luaL_checkstring(L, 2), NULL, 0);
    
    lua_pop(L, 2);
    
    ud_set_pc(ud_obj, rip);
    
    return 0;
}


int ludis86_ud_set_syntax (lua_State * L)
{
    ud_t * ud_obj;
    const char * syntax;
    
    ud_obj = ludis86_check_ud_t(L, 1);
    syntax = luaL_checkstring(L, 2);
    
    lua_pop(L, 2);
    
    if (strcmp(syntax, "UD_SYN_INTEL") == 0)
        ud_set_syntax(ud_obj, UD_SYN_INTEL);
    else if (strcmp(syntax, "UD_SYN_ATT") == 0)
        ud_set_syntax(ud_obj, UD_SYN_ATT);
    else
        luaL_error(L, "invalid syntax for ludis86");
    
    return 0;
}


int ludis86_ud_set_vendor (lua_State * L)
{
    ud_t * ud_obj;
    const char * vendor;
    
    ud_obj = ludis86_check_ud_t(L, 1);
    vendor = luaL_checkstring(L, 2);
    
    lua_pop(L, 2);
    
    if (strcmp(vendor, "UD_VENDOR_INTEL") == 0)
        ud_set_vendor(ud_obj, UD_VENDOR_INTEL);
    else if (strcmp(vendor, "UD_VENDOR_ATT") == 0)
        ud_set_vendor(ud_obj, UD_VENDOR_AMD);
    else
        luaL_error(L, "invalid vendor for ludis86");
    
    return 0;
}


int ludis86_ud_disassemble (lua_State * L)
{
    ud_t * ud_obj;
    int bytes_disassembled;
    
    ud_obj = ludis86_check_ud_t(L, 1);
    
    lua_pop(L, 1);
    
    bytes_disassembled = ud_disassemble(ud_obj);
    
    lua_pushinteger(L, bytes_disassembled);
    
    return 1;
}


int ludis86_ud_insn_len (lua_State * L)
{
    ud_t * ud_obj;
    
    ud_obj = ludis86_check_ud_t(L, 1);
    
    lua_pop(L, 1);
    
    lua_pushinteger(L, ud_insn_len(ud_obj));
    
    return 1;
}


int ludis86_ud_insn_off (lua_State * L)
{
    ud_t * ud_obj;
    
    ud_obj = ludis86_check_ud_t(L, 1);
    
    lua_pop(L, 1);
    
    lua_pushinteger(L, ud_insn_off(ud_obj));
    
    return 1;
}


int ludis86_ud_insn_hex (lua_State * L)
{
    ud_t * ud_obj;
    
    ud_obj = ludis86_check_ud_t(L, 1);
    
    lua_pop(L, 1);
    
    lua_pushinteger(L, ud_insn_off(ud_obj));
    
    return 1;
}


int ludis86_ud_insn_ptr (lua_State * L)
{
    ud_t * ud_obj;
    
    ud_obj = ludis86_check_ud_t(L, 1);
    
    lua_pop(L, 1);
    
    lua_pushlstring(L, ud_insn_ptr(ud_obj), ud_insn_len(ud_obj));
    
    return 1;
}


int ludis86_ud_insn_asm (lua_State * L)
{
    ud_t * ud_obj;
    
    ud_obj = ludis86_check_ud_t(L, 1);
    
    lua_pop(L, 1);
    
    lua_pushstring(L, ud_insn_asm(ud_obj));
    
    return 1;
}


int ludis86_ud_input_skip (lua_State * L)
{
    ud_t * ud_obj;
    int skip_bytes;
    
    ud_obj     = ludis86_check_ud_t(L, 1);
    skip_bytes = luaL_checkint(L, 2);
    
    lua_pop(L, 2);
    
    ud_input_skip(ud_obj, skip_bytes);
    
    return 0;
}
