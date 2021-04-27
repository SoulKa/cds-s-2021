#define _GNU_SOURCE

#include <pthread.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sched.h>
#include <string.h>

#ifdef MEASURE_TIMING
#include "common.h"
#endif

// TYPEDEFS
typedef unsigned int uint;
typedef unsigned char byte;

typedef struct {
    byte padding_front[64];
    uint thread_number;
    uint rows;
    uint cols;
    float rows_f;
    float cols_f;
    uint num_iterations;
    u_char num_cores;
    char *img_part;
    uint _p_begin;
    uint _p_end;
    uint _p;
    uint _n;
    uint _row;
    uint _col;
    float _i_iterator;
    float _r_iterator;
    float _z_r;
    float _z_i;
    float _z_r_tmp;
    byte padding_back[64];
} mandelbrot_params_t;

// GLOBAL VARS

#ifdef MEASURE_TIMING
clock_t ts_begin;
clock_t ts_calculation;

clock_t time_full;
clock_t time_preparation;
clock_t time_calculation;

clock_t *time_threads;
#endif

// FUNCTIONS

void *work( void *params_uncasted )
{

    #ifdef MEASURE_TIMING
    clock_t ts_begin_thread = get_timestamp();
    #endif

    // cast to function parameters and variables
    mandelbrot_params_t* params = (mandelbrot_params_t*) params_uncasted;

    // set CPU affinity
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(params->thread_number, &cpuset);
    if (pthread_setaffinity_np(pthread_self(), sizeof(cpuset), &cpuset) != 0) {
        fprintf(stderr, "Could not set CPU affinity for thread %u (handle %lu)!\n", params->thread_number, pthread_self());
    } else {
        fprintf(stderr, "Running thread %u on CPU %d\n", params->thread_number, sched_getcpu());
    }

    params->_p_begin = params->thread_number*params->rows*(params->cols+1u)/params->num_cores;
    params->_p_end = (params->thread_number+1u)*params->rows*(params->cols+1u)/params->num_cores;
    params->_row = params->_p_begin / (params->cols+1u);
    params->_col = params->_p_begin % (params->cols+1u);

    params->img_part = malloc((params->_p_end - params->_p_begin + 128u) * sizeof(params->img_part[0]))+64lu; // add 64byte before and after to make sure that no cache miss will happen
    //memset(params->img_part-64lu, 0, (params->_p_end - params->_p_begin + 128u) * sizeof(params->img_part[0]));

    // iterate over all pixel that this thread has to calculate
    //for (uint p = thread_number; p < rows*(cols+1u); p += NUM_CORES) {
    for (params->_p = params->_p_begin; params->_p < params->_p_end; params->_p++) {

        if (params->_col < params->cols) {

            // prepare variables for next iteration
            params->_z_r = 0.0f;
            params->_z_i = 0.0f;
            params->_r_iterator = params->_col * 2.0f / params->cols_f - 1.5f;
            params->_i_iterator = params->_row * 2.0f / params->rows_f - 1.0f;

            // calculate next pixel
            for (params->_n=0u; params->_n < params->num_iterations; params->_n++) {

                // square z and add offset
                params->_z_r_tmp = 2.0f*params->_z_r;
                params->_z_r = params->_z_r*params->_z_r - params->_z_i*params->_z_i + params->_r_iterator;
                params->_z_i = params->_z_i * params->_z_r_tmp + params->_i_iterator;

            }

            // set pixel
            params->img_part[params->_p-params->_p_begin] = sqrtf(params->_z_r*params->_z_r + params->_z_i*params->_z_i) < 2.0f ? '#' : '.';
            params->_col++;

        } else {

            // insert newline
            params->img_part[params->_p-params->_p_begin] = '\n';
            params->_col = 0u;
            params->_row++;

        }

    }

    #ifdef MEASURE_TIMING
    time_threads[params->thread_number] = get_timediff(ts_begin_thread);
    #endif

    return NULL;

}

int main() {

    #ifdef MEASURE_TIMING
    ts_begin = get_timestamp();
    #endif

    // get amount of cores
    const char *NUM_CORES_STRING = getenv("MAX_CPUS");
    ushort NUM_CORES = NUM_CORES_STRING == NULL ? 1 : strtoul(NUM_CORES_STRING, NULL, 10);
    fprintf(stderr, "Working with %u threads\n", NUM_CORES);

    // read stdin
    uint rows, cols, max_iterations;
    (void)! scanf("%u", &rows);
    (void)! scanf("%u", &cols);
    (void)! scanf("%u", &max_iterations);

    #ifdef MEASURE_TIMING
    time_preparation = get_timediff(ts_begin);
    ts_calculation = get_timestamp();
    time_threads = malloc(NUM_CORES * sizeof(clock_t));
    #endif

    // calculate mandelbrot set
    pthread_t *threads = malloc(NUM_CORES * sizeof(pthread_t));
    mandelbrot_params_t *params_arr = malloc(NUM_CORES * sizeof(mandelbrot_params_t));
    uint i;
    for (i = 0u; i < NUM_CORES; i++) {

        params_arr[i].thread_number = i;
        params_arr[i].rows = rows;
        params_arr[i].cols = cols;
        params_arr[i].rows_f = rows;
        params_arr[i].cols_f = cols;
        params_arr[i].num_iterations = max_iterations;
        params_arr[i].num_cores = NUM_CORES;
        
        if (i == NUM_CORES-1)
            work(&params_arr[i]);
        else
            pthread_create(&threads[i], NULL, work, &params_arr[i]);

    }

    // wait for them to finish
    for (i = 0u; i < NUM_CORES; i++) {

        if (i != NUM_CORES-1) pthread_join(threads[i], NULL);

        // print result
        fwrite(params_arr[i].img_part, 1, (i+1)*rows*(cols+1)/NUM_CORES-i*rows*(cols+1)/NUM_CORES, stdout);

    }

    #ifdef MEASURE_TIMING
    time_calculation = get_timediff(ts_calculation);
    ts_calculation = get_timestamp();
    time_full = get_timediff(ts_begin);

    fprintf(stderr, "Time full: %.3fms\n", time_full/1.0e4);
    fprintf(stderr, "Time preparation: %.3fms (%.2f%%)\n", time_preparation/1.0e4, time_preparation*100.0/time_full);
    fprintf(stderr, "Time mandelbrot: %.3fms (%.2f%%)\n", time_calculation/1.0e4, time_calculation*100.0/time_full);

    for (uint i = 0u; i < NUM_CORES; i++) fprintf(stderr, "Time thread %u: %.3fms\n", i, time_threads[i]/1.0e4);
    #endif

    return 0;

}


