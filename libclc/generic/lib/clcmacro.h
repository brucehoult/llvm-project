#define _CLC_UNARY_VECTORIZE(DECLSPEC, RET_TYPE, FUNCTION, ARG1_TYPE) \
  DECLSPEC RET_TYPE##2 FUNCTION(ARG1_TYPE##2 x) { \
    return (RET_TYPE##2)(FUNCTION(x.x), FUNCTION(x.y)); \
  } \
\
  DECLSPEC RET_TYPE##3 FUNCTION(ARG1_TYPE##3 x) { \
    return (RET_TYPE##3)(FUNCTION(x.x), FUNCTION(x.y), FUNCTION(x.z)); \
  } \
\
  DECLSPEC RET_TYPE##4 FUNCTION(ARG1_TYPE##4 x) { \
    return (RET_TYPE##4)(FUNCTION(x.lo), FUNCTION(x.hi)); \
  } \
\
  DECLSPEC RET_TYPE##8 FUNCTION(ARG1_TYPE##8 x) { \
    return (RET_TYPE##8)(FUNCTION(x.lo), FUNCTION(x.hi)); \
  } \
\
  DECLSPEC RET_TYPE##16 FUNCTION(ARG1_TYPE##16 x) { \
    return (RET_TYPE##16)(FUNCTION(x.lo), FUNCTION(x.hi)); \
  }

#define _CLC_BINARY_VECTORIZE(DECLSPEC, RET_TYPE, FUNCTION, ARG1_TYPE, ARG2_TYPE) \
  DECLSPEC RET_TYPE##2 FUNCTION(ARG1_TYPE##2 x, ARG2_TYPE##2 y) { \
    return (RET_TYPE##2)(FUNCTION(x.x, y.x), FUNCTION(x.y, y.y)); \
  } \
\
  DECLSPEC RET_TYPE##3 FUNCTION(ARG1_TYPE##3 x, ARG2_TYPE##3 y) { \
    return (RET_TYPE##3)(FUNCTION(x.x, y.x), FUNCTION(x.y, y.y), \
                         FUNCTION(x.z, y.z)); \
  } \
\
  DECLSPEC RET_TYPE##4 FUNCTION(ARG1_TYPE##4 x, ARG2_TYPE##4 y) { \
    return (RET_TYPE##4)(FUNCTION(x.lo, y.lo), FUNCTION(x.hi, y.hi)); \
  } \
\
  DECLSPEC RET_TYPE##8 FUNCTION(ARG1_TYPE##8 x, ARG2_TYPE##8 y) { \
    return (RET_TYPE##8)(FUNCTION(x.lo, y.lo), FUNCTION(x.hi, y.hi)); \
  } \
\
  DECLSPEC RET_TYPE##16 FUNCTION(ARG1_TYPE##16 x, ARG2_TYPE##16 y) { \
    return (RET_TYPE##16)(FUNCTION(x.lo, y.lo), FUNCTION(x.hi, y.hi)); \
  }

#define _CLC_V_S_V_VECTORIZE(DECLSPEC, RET_TYPE, FUNCTION, ARG1_TYPE, ARG2_TYPE) \
  DECLSPEC RET_TYPE##2 FUNCTION(ARG1_TYPE x, ARG2_TYPE##2 y) { \
    return (RET_TYPE##2)(FUNCTION(x, y.lo), FUNCTION(x, y.hi)); \
  } \
\
  DECLSPEC RET_TYPE##3 FUNCTION(ARG1_TYPE x, ARG2_TYPE##3 y) { \
    return (RET_TYPE##3)(FUNCTION(x, y.x), FUNCTION(x, y.y), \
                         FUNCTION(x, y.z)); \
  } \
\
  DECLSPEC RET_TYPE##4 FUNCTION(ARG1_TYPE x, ARG2_TYPE##4 y) { \
    return (RET_TYPE##4)(FUNCTION(x, y.lo), FUNCTION(x, y.hi)); \
  } \
\
  DECLSPEC RET_TYPE##8 FUNCTION(ARG1_TYPE x, ARG2_TYPE##8 y) { \
    return (RET_TYPE##8)(FUNCTION(x, y.lo), FUNCTION(x, y.hi)); \
  } \
\
  DECLSPEC RET_TYPE##16 FUNCTION(ARG1_TYPE x, ARG2_TYPE##16 y) { \
    return (RET_TYPE##16)(FUNCTION(x, y.lo), FUNCTION(x, y.hi)); \
  } \
\

#define _CLC_TERNARY_VECTORIZE(DECLSPEC, RET_TYPE, FUNCTION, ARG1_TYPE, ARG2_TYPE, ARG3_TYPE) \
  DECLSPEC RET_TYPE##2 FUNCTION(ARG1_TYPE##2 x, ARG2_TYPE##2 y, ARG3_TYPE##2 z) { \
    return (RET_TYPE##2)(FUNCTION(x.x, y.x, z.x), FUNCTION(x.y, y.y, z.y)); \
  } \
\
  DECLSPEC RET_TYPE##3 FUNCTION(ARG1_TYPE##3 x, ARG2_TYPE##3 y, ARG3_TYPE##3 z) { \
    return (RET_TYPE##3)(FUNCTION(x.x, y.x, z.x), FUNCTION(x.y, y.y, z.y), \
                         FUNCTION(x.z, y.z, z.z)); \
  } \
\
  DECLSPEC RET_TYPE##4 FUNCTION(ARG1_TYPE##4 x, ARG2_TYPE##4 y, ARG3_TYPE##4 z) { \
    return (RET_TYPE##4)(FUNCTION(x.lo, y.lo, z.lo), FUNCTION(x.hi, y.hi, z.hi)); \
  } \
\
  DECLSPEC RET_TYPE##8 FUNCTION(ARG1_TYPE##8 x, ARG2_TYPE##8 y, ARG3_TYPE##8 z) { \
    return (RET_TYPE##8)(FUNCTION(x.lo, y.lo, z.lo), FUNCTION(x.hi, y.hi, z.hi)); \
  } \
\
  DECLSPEC RET_TYPE##16 FUNCTION(ARG1_TYPE##16 x, ARG2_TYPE##16 y, ARG3_TYPE##16 z) { \
    return (RET_TYPE##16)(FUNCTION(x.lo, y.lo, z.lo), FUNCTION(x.hi, y.hi, z.hi)); \
  }

#define _CLC_V_S_S_V_VECTORIZE(DECLSPEC, RET_TYPE, FUNCTION, ARG1_TYPE, ARG2_TYPE, ARG3_TYPE) \
  DECLSPEC RET_TYPE##2 FUNCTION(ARG1_TYPE x, ARG2_TYPE y, ARG3_TYPE##2 z) { \
    return (RET_TYPE##2)(FUNCTION(x, y, z.lo), FUNCTION(x, y, z.hi)); \
  } \
\
  DECLSPEC RET_TYPE##3 FUNCTION(ARG1_TYPE x, ARG2_TYPE y, ARG3_TYPE##3 z) { \
    return (RET_TYPE##3)(FUNCTION(x, y, z.x), FUNCTION(x, y, z.y), \
                         FUNCTION(x, y, z.z)); \
  } \
\
  DECLSPEC RET_TYPE##4 FUNCTION(ARG1_TYPE x, ARG2_TYPE y, ARG3_TYPE##4 z) { \
    return (RET_TYPE##4)(FUNCTION(x, y, z.lo), FUNCTION(x, y, z.hi)); \
  } \
\
  DECLSPEC RET_TYPE##8 FUNCTION(ARG1_TYPE x, ARG2_TYPE y, ARG3_TYPE##8 z) { \
    return (RET_TYPE##8)(FUNCTION(x, y, z.lo), FUNCTION(x, y, z.hi)); \
  } \
\
  DECLSPEC RET_TYPE##16 FUNCTION(ARG1_TYPE x, ARG2_TYPE y, ARG3_TYPE##16 z) { \
    return (RET_TYPE##16)(FUNCTION(x, y, z.lo), FUNCTION(x, y, z.hi)); \
  } \
\

#define _CLC_V_V_VP_VECTORIZE(DECLSPEC, RET_TYPE, FUNCTION, ARG1_TYPE, ARG2_TYPE) \
  DECLSPEC RET_TYPE##2 FUNCTION(ARG1_TYPE##2 x, ARG2_TYPE##2 *y) { \
    return (RET_TYPE##2)(FUNCTION(x.x, (ARG2_TYPE*)y), FUNCTION(x.y, (ARG2_TYPE*)y+1)); \
  } \
\
  DECLSPEC RET_TYPE##3 FUNCTION(ARG1_TYPE##3 x, ARG2_TYPE##3 *y) { \
    return (RET_TYPE##3)(FUNCTION(x.x, (ARG2_TYPE*)y), FUNCTION(x.y, (ARG2_TYPE*)y+1), \
                         FUNCTION(x.z, (ARG2_TYPE*)y+2)); \
  } \
\
  DECLSPEC RET_TYPE##4 FUNCTION(ARG1_TYPE##4 x, ARG2_TYPE##4 *y) { \
    return (RET_TYPE##4)(FUNCTION(x.lo, (ARG2_TYPE##2*)y), FUNCTION(x.hi, (ARG2_TYPE##2*)((ARG2_TYPE*)y+2))); \
  } \
\
  DECLSPEC RET_TYPE##8 FUNCTION(ARG1_TYPE##8 x, ARG2_TYPE##8 *y) { \
    return (RET_TYPE##8)(FUNCTION(x.lo, (ARG2_TYPE##4*)y), FUNCTION(x.hi, (ARG2_TYPE##4*)((ARG2_TYPE*)y+4))); \
  } \
\
  DECLSPEC RET_TYPE##16 FUNCTION(ARG1_TYPE##16 x, ARG2_TYPE##16 *y) { \
    return (RET_TYPE##16)(FUNCTION(x.lo, (ARG2_TYPE##8*)y), FUNCTION(x.hi, (ARG2_TYPE##8*)((ARG2_TYPE*)y+8))); \
  }

#define _CLC_DEFINE_BINARY_BUILTIN(RET_TYPE, FUNCTION, BUILTIN, ARG1_TYPE, ARG2_TYPE) \
_CLC_DEF _CLC_OVERLOAD RET_TYPE FUNCTION(ARG1_TYPE x, ARG2_TYPE y) { \
  return BUILTIN(x, y); \
} \
_CLC_BINARY_VECTORIZE(_CLC_OVERLOAD _CLC_DEF, RET_TYPE, FUNCTION, ARG1_TYPE, ARG2_TYPE)

#define _CLC_DEFINE_BINARY_BUILTIN_WITH_SCALAR_SECOND_ARG(RET_TYPE, FUNCTION, BUILTIN, ARG1_TYPE, ARG2_TYPE) \
_CLC_DEFINE_BINARY_BUILTIN(RET_TYPE, FUNCTION, BUILTIN, ARG1_TYPE, ARG2_TYPE) \
_CLC_BINARY_VECTORIZE_SCALAR_SECOND_ARG(_CLC_OVERLOAD _CLC_DEF, RET_TYPE, FUNCTION, ARG1_TYPE, ARG2_TYPE)

#define _CLC_DEFINE_UNARY_BUILTIN(RET_TYPE, FUNCTION, BUILTIN, ARG1_TYPE) \
_CLC_DEF _CLC_OVERLOAD RET_TYPE FUNCTION(ARG1_TYPE x) { \
  return BUILTIN(x); \
} \
_CLC_UNARY_VECTORIZE(_CLC_OVERLOAD _CLC_DEF, RET_TYPE, FUNCTION, ARG1_TYPE)
