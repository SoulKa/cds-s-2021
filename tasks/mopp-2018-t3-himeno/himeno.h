#ifndef __HEADER_HIMENO__
#define __HEADER_HIMENO__

#include "vector.h"
#include "matrix.h"
#include "common.h"

#ifdef USE_FLOAT64
    #define FLOAT_TYPE_TO_USE double
#else
    #define FLOAT_TYPE_TO_USE float
#endif

typedef Vector3<uint> vec3_uint_t;
typedef Vector4<uint> vec4_uint_t;

FLOAT_TYPE_TO_USE jacobi( uint nn );

#endif