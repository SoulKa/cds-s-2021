#ifndef __HEADER_HIMENO__
#define __HEADER_HIMENO__

#include "vector.h"
#include "matrix.h"
#include "common.h"

// TYPEDEFS
typedef Vector3<uint> vec3_uint_t;
typedef Vector4<uint> vec4_uint_t;

typedef struct {
    Matrix a;
    Matrix b;
    Matrix c;
    Matrix p;
    Matrix bnd;
    Matrix wrk1;
    Matrix wrk2;
} matrix_set_t;

double jacobi( uint nn, matrix_set_t *matrices );

#endif