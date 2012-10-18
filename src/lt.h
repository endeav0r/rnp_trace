#ifndef lt_HEADER
#define lt_HEADER

#include "tracer.h"

int lt_tracer_new   (lua_State * L);
int lt_tracer_gc    (lua_State * L);

struct _tracer * lt_tracer_check_t (lua_State * L, int position);

int lt_tracer_step (lua_State * L);
int lt_tracer_term (lua_State * L);
int lt_tracer_ip   (lua_State * L);
int lt_tracer_pid  (lua_State * L);
int lt_tracer_mem  (lua_State * L);

#endif