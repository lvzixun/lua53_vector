#ifndef lvector_h
#define lvector_h

#include "lobject.h"

// not need
// #define sizevector sizeof(TVector)

struct vector_env;
LUAI_FUNC struct vector_env* lvector_env_new(lua_State* L);
LUAI_FUNC void lvector_env_free(lua_State* L, struct vector_env* env);

LUAI_FUNC TVector *luaVT_new(lua_State *L);
LUAI_FUNC void luaVT_free(lua_State* L, TVector* vt);
TVector* luaVT_clone(TVector* v1, TVector* result);

// vector op
TVector* luaVT_add(TVector* v1, TVector* v2, TVector* result);
TVector* luaVT_sub(TVector* v1, TVector* v2, TVector* result);
double luaVT_mul(TVector* v1, TVector* v2);
double luaVT_distance(TVector* v1, TVector* v2);
TVector* luaVT_scale(TVector* v1, double k, TVector* result);

#endif