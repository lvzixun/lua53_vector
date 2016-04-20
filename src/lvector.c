#include "lua.h"

#include "lvector.h"
#include "lstate.h"
#include "lobject.h"
#include "lgc.h"

#include <math.h>
#include <stdio.h>
#include <string.h>

#define DEF_PAGE_SIZE 128

struct node {
  union {
    TVector vt;
    struct node* next;
  }v;
};

struct page {
  struct node slot[DEF_PAGE_SIZE];
  struct page* next;
};

struct vector_env {
  struct page* pool;
  struct node* free_head;
};

#define  _G_VT_ENV(L) (G(L)->vt_env)

static struct page* _page_new(lua_State* L);
static void _page_free(lua_State* L, struct page* p);


struct vector_env* lvector_env_new(lua_State* L) {
  struct vector_env* ret = luaM_new(L, struct vector_env);
  
  ret->pool = _page_new(L);
  ret->free_head = ret->pool->slot;
  return ret;
}

void lvector_env_free(lua_State* L, struct vector_env* env) {
  struct page* p = env->pool;
  
  while(p) {
    struct page* next = p->next;
    _page_free(L, p);
    p = next;
  }

  luaM_free(L, env);
}


static struct page* _page_new(lua_State* L) {
  struct page* _page = luaM_new(L, struct page);
  size_t i;
  size_t sz = sizeof(_page->slot) / sizeof(_page->slot[0]);

  for(i=0; i<sz-1; i++) {
    _page->slot[i].v.next = _page->slot + i + 1;
  }
  _page->slot[sz-1].v.next = NULL;
  _page->next = NULL;
  return _page;
}


static void _page_free(lua_State* L, struct page* p) {
  luaM_free(L, p);
}


static void pool_resize(lua_State* L, struct vector_env* env) {
  if(env->free_head)
    return;

  struct page* new_page = _page_new(L);
  new_page->next = env->pool;
  env->pool = new_page;
  env->free_head = new_page->slot;
}


static TVector* pool_get(lua_State* L, struct vector_env* env) {
  pool_resize(L, env);
  TVector* ret = &(env->free_head->v.vt);
  lua_assert((void*)(ret) == (void*)(env->free_head));
  env->free_head = env->free_head->v.next;
  return ret;
}

static inline double fast_inv_sqrt(double x) {
  double y = x;
  double xhalf = ( double )0.5 * y;
  long long i = *( long long* )( &y );
  i = 0x5fe6ec85e7de30daLL - ( i >> 1 );//LL suffix for (long long) type for GCC
  y = *( double* )( &i );
  y = y * ( ( double )1.5 - xhalf * y * y );
  return y;
}


TVector* luaVT_new(lua_State *L) {
  global_State *g = G(L);
  struct vector_env* env = _G_VT_ENV(L);
  TVector* v = pool_get(L, env);
  int i;
  for(i=0; i<VECTOR_ELEMENT_LEN; i++) {
    v->elements[i] = 0.0;
  }
  GCObject* o = obj2gco(v);
  o->marked = luaC_white(g);
  o->tt = LUA_TVECTOR;
  o->next = g->allgc;
  g->allgc = o;
  return v;
}


void luaVT_free(lua_State* L, TVector* vt) {
  struct vector_env* env = _G_VT_ENV(L);
  struct node* free_node = cast(struct node*, vt);
  free_node->v.next = env->free_head;
  env->free_head = free_node;
}


TVector* luaVT_clone(TVector* v1, TVector* result) {
  memcpy(result->elements, v1->elements, sizeof(v1->elements));
  return v1;
}


TVector* luaVT_add(TVector* v1, TVector* v2, TVector* result) {
  int i;
  for(i=0; i<VECTOR_ELEMENT_LEN; i++) {
    result->elements[i] = v1->elements[i] + v2->elements[i];
  }
  return result;
}

TVector* luaVT_sub(TVector* v1, TVector* v2, TVector* result) {
  int i;
  for(i=0; i<VECTOR_ELEMENT_LEN; i++) {
    result->elements[i] = v1->elements[i] - v2->elements[i];
  }
  return result;
}

double luaVT_mul(TVector* v1, TVector* v2) {
  int i;
  double r = 0.0;
  for(i=0; i<VECTOR_ELEMENT_LEN; i++) {
    double v = v1->elements[i] * v2->elements[i];
    r += v; 
  }
  return r;
}

double luaVT_distance(TVector* v1, TVector* v2) {
  double r = luaVT_mul(v1, v2);
  return 1/fast_inv_sqrt(r);
}


TVector* luaVT_scale(TVector* v1, double k, TVector* result) {
  int i;
  for(i=0; i<VECTOR_ELEMENT_LEN; i++) {
    result->elements[i] = v1->elements[i] * k;
  }
  return result;
}

