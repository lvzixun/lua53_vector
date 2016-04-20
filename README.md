# lua53_vector
  add vector type to lua5.3, through the new `OP_VT_IADD, OP_VT_ISUB, OP_VT_IMUL ...` opcode to vector in-place arithmetic operation.

## syntax
create vector
```
  create_vector_exp ->
    '@' 'V'| 'v' '(' explist ')'
```

clone vector
```
  clone_vector_exp ->
    '@' 'C' | 'c' '(' explist ')'
```

in-place arithmetic operation
```
  vector_add_exp ->
    '@' '+' '(' suffixedexp ',' subexpr ')'

  vector_sub_exp ->
    '@' '-' '(' suffixedexp ',' subexpr ')'

  vector_mul_exp ->
    '@' '*' '(' suffixedexp ',' subexpr ')'

  vector_distance_exp ->
    '@' '^' '(' suffixedexp ',' subexpr ')'
```

### tutorial
~~~.lua
  local v1, v2 = @v(1, 2, 3), @v(4, 5, 6) -- create v1 and v2 vector
  @+(v1, v2)                              -- v1 = v1 + v2
  @-(v1, v2)                              -- v1 = v1 - v2
  @*(v1, v2)                              -- v1 = v1 * v2
  @^(v1, v2)                              -- v1 = (v1 * v2)^0.5

  @c(v1, v2)                              -- clone v2 to v1
~~~
you also use `local v = v1 + v2` create a new `v` vector object.

get elements of vector:
~~~.lua
  local v = @v(11, 22, 33)
  print(v[1], v[2], v[3]) -- output: 11, 22, 33
~~~

vector unpack function:
~~~.lua
  local v = @v(11, 22, 33)
  local x, y, z = vtunpack(v)
  print(x, y, z) -- output: 11, 22, 33
~~~

### C api
~~~.c
  #define VECTOR_ELEMENT_LEN 4
  typedef struct {
    double elements[VECTOR_ELEMENT_LEN];
  } lua_vector;

  lua_vector* lua_tovector  (lua_State *L, int idx, lua_vector* out_vt);   // [-0, +0, -]
  void        lua_newvector (lua_State* L, lua_vector* v);                 // [-0, +1, e]
~~~
read [test_api.c](https://github.com/lvzixun/lua53_vector/blob/master/test/test_api.c) for detail.




## benchmark
in my i5@2.4GHz CPU:

| target | excute 60M |
|:------:|:----------:|
| vector type | 0.908S |
| [vector lib](https://github.com/lvzixun/lua53_vector/blob/master/test/vector.c) | 5.292S |