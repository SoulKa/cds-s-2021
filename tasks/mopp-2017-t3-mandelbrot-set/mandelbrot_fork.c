#define _GNU_SOURCE

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sched.h>
#include <string.h>

// TYPEDEFS
typedef unsigned int uint;
typedef unsigned char byte;

typedef struct {
    byte padding_front[64];
    ushort rows;
    ushort cols;
    float rows_f;
    float cols_f;
    ushort num_iterations;
    char *img_part;
    uint p_begin;
    uint p_end;
    uint _p;
    ushort _n;
    ushort _row;
    ushort _col;
    float _i_iterator;
    float _r_iterator;
    float _z_r;
    float _z_i;
    float _z_r_tmp;
    byte padding_back[64];
} mandelbrot_params_t;

int main( int arc, char *argv[] ) {

    mandelbrot_params_t params;

    // read stdin
    (void)! scanf("%hu", &params.rows);
    (void)! scanf("%hu", &params.cols);
    (void)! scanf("%hu", &params.num_iterations);
    params.rows_f = params.rows;
    params.cols_f = params.cols;

    u_char worker_id = strtoul(argv[1], NULL, 10);
    u_char num_workers = strtoul(argv[2], NULL, 10);

    params.p_begin = worker_id*params.rows*params.cols/num_workers;
    params.p_end = (worker_id+1)*params.rows*params.cols/num_workers;

    params._row = params.p_begin / params.cols;
    params._col = params.p_begin % params.cols;

    params.img_part = malloc((params.p_end - params.p_begin + 128u) * sizeof(params.img_part[0]))+64lu; // add 64byte before and after to make sure that no cache miss will happen
    //memset(params.img_part-64lu, 0, (params.p_end - params.p_begin + 128u) * sizeof(params.img_part[0]));

    // iterate over all pixel that this thread has to calculate
    for (params._p = params.p_begin; params._p < params.p_end; params._p++) {

        // insert newline on column end
        if (params._col == params.cols) {
            fputc('\n', stdout);
            params._col = 0u;
            params._row++;
        }

        // prepare variables for next iteration
        params._z_r = 0.0f;
        params._z_i = 0.0f;
        params._r_iterator = params._col * 2.0f / params.cols_f - 1.5f;
        params._i_iterator = params._row * 2.0f / params.rows_f - 1.0f;

        // calculate next pixel
        for (params._n=0u; params._n < params.num_iterations; params._n++) {

            // square z and add offset
            params._z_r_tmp = 2.0f*params._z_r;
            params._z_r = params._z_r*params._z_r - params._z_i*params._z_i + params._r_iterator;
            params._z_i = params._z_i * params._z_r_tmp + params._i_iterator;

        }

        // set pixel
        fputc( sqrtf(params._z_r*params._z_r + params._z_i*params._z_i) < 2.0f ? '#' : '.', stdout );
        params._col++;

    }

    if (params._col == params.cols) fputc('\n', stdout);

    // done
    return 0;

}


