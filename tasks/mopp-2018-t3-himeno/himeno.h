#ifndef __HEADER_HIMENO__
#define __HEADER_HIMENO__

#include "vector.h"
#include "matrix.h"
#include "common.h"

// TYPEDEFS
typedef Vector3<uint> vec3_uint_t;
typedef Vector4<uint> vec4_uint_t;
typedef Matrix<double> mat_float64_t;

typedef struct {
    mat_float64_t a;
    mat_float64_t b;
    mat_float64_t c;
    mat_float64_t p;
    mat_float64_t bnd;
    mat_float64_t wrk1;
    mat_float64_t wrk2;
} matrix_set_t;

double jacobi( uint nn, matrix_set_t *matrices );

#endif