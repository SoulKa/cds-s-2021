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
    uint value;
    pthread_mutex_t mutex;
} atomic_uint_t;

typedef struct {
    byte padding_front[64];
    uint thread_number;
    uint rows;
    uint cols;
    float rows_f;
    float cols_f;
    uint num_iterations;
    u_char num_cores;
    atomic_uint_t *next_pixel;
    char *img;
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
    mandelbrot_params_t* p = (mandelbrot_params_t*) params_uncasted;

    // set CPU affinity
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(p->thread_number, &cpuset);
    if (pthread_setaffinity_np(pthread_self(), sizeof(cpuset), &cpuset) != 0) {
        fprintf(stderr, "Could not set CPU affinity for thread %u (handle %lu)!\n", p->thread_number, pthread_self());
    } else {
        fprintf(stderr, "Running thread %u on CPU %d\n", p->thread_number, sched_getcpu());
    }

    p->_p_begin = p->thread_number*p->rows*(p->cols+1u)/(p->num_cores*2u);
    p->_p_end = (p->thread_number+1u)*p->rows*(p->cols+1u)/(p->num_cores*2u);
    
    do {

        p->_row = p->_p_begin / (p->cols+1u);
        p->_col = p->_p_begin % (p->cols+1u);

        // iterate over all pixel that this thread has to calculate
        for (p->_p = p->_p_begin; p->_p < p->_p_end; p->_p++) {

            if (p->_col < p->cols) {

                // prepare variables for next iteration
                p->_z_r = 0.0f;
                p->_z_i = 0.0f;
                p->_r_iterator = p->_col * 2.0f / p->cols_f - 1.5f;
                p->_i_iterator = p->_row * 2.0f / p->rows_f - 1.0f;

                // calculate next pixel
                for (p->_n=0u; p->_n < p->num_iterations; p->_n++) {

                    // square z and add offset
                    p->_z_r_tmp = 2.0f*p->_z_r;
                    p->_z_r = p->_z_r*p->_z_r - p->_z_i*p->_z_i + p->_r_iterator;
                    p->_z_i = p->_z_i * p->_z_r_tmp + p->_i_iterator;

                }

                // set pixel
                p->img[p->_p] = sqrtf(p->_z_r*p->_z_r + p->_z_i*p->_z_i) < 2.0f ? '#' : '.';
                p->_col++;

            } else {

                // insert newline
                p->img[p->_p] = '\n';
                p->_col = 0u;
                p->_row++;

            }

        }

        // get more work
        pthread_mutex_lock(&p->next_pixel->mutex);
        p->_p_end = (p->next_pixel->value += 10u);
        pthread_mutex_unlock(&p->next_pixel->mutex);
        p->_p_begin = p->_p_end - 10u;
        if (p->_p_end > p->rows*(p->cols+1u)) p->_p_end = p->rows*(p->cols+1u);

    } while(p->_p_begin < p->_p_end);

    #ifdef MEASURE_TIMING
    time_threads[p->thread_number] = get_timediff(ts_begin_thread);
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
    char *img = malloc(rows*(cols+1) + 128lu) + 64lu;
    atomic_uint_t worker_pixel = { 0u };
    pthread_mutex_init(&worker_pixel.mutex, 0);

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
        params_arr[i].img = img;
        params_arr[i].next_pixel = &worker_pixel;
        
        if (i == NUM_CORES-1)
            work(&params_arr[i]);
        else
            pthread_create(&threads[i], NULL, work, &params_arr[i]);

    }

    // wait for them to finish
    for (i = 0u; i < NUM_CORES; i++) if (i != NUM_CORES-1) pthread_join(threads[i], NULL);

    // print result
    fwrite(img, 1, rows*(cols+1), stdout);

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


