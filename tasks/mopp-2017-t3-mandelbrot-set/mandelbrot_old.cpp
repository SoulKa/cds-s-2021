#include <pthread.h>
#include <math.h>
#include <stdio.h>
#include <time.h>

#ifdef MEASURE_TIMING
#include "common.h"
#endif

using namespace std;

// TYPEDEFS
typedef unsigned int uint;
typedef unsigned char byte;

struct mandelbrot_params_t {
    byte padding_front[64];
    uint thread_number;
    uint rows;
    uint cols;
    uint num_iterations;
    char *img_part;
    uint _p_begin;
    uint _p_end;
    uint _p;
    uint _n;
    uint _row;
    uint _col;
    float _z_r;
    float _z_i;
    float _z_r_tmp;
    byte padding_back[64];
};

// GLOBAL VARS
unsigned long NUM_CORES;

pthread_mutex_t current_row_mutex;

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
    auto ts_begin_thread = get_timestamp();
    #endif

    auto params = (mandelbrot_params_t*) params_uncasted;

    params->_p_begin = params->thread_number*params->rows*(params->cols+1u)/NUM_CORES;
    params->_p_end = (params->thread_number+1u)*params->rows*(params->cols+1u)/NUM_CORES;

    params->img_part = new char[params->_p_end-params->_p_begin+128u]+64u; // add 64byte before and after to make sure that no cache miss will happen
    //memset(img_part, 0, p_end-p_begin);

    // iterate over all pixel that this thread has to calculate
    //for (uint p = thread_number; p < rows*(cols+1u); p += NUM_CORES) {
    for (params->_p = params->_p_begin; params->_p < params->_p_end; params->_p++) {

        params->_row = params->_p / (params->cols+1u);
        params->_col = params->_p % (params->cols+1u);

        if (params->_col < params->cols) {

            // prepare variables for next iteration
            params->_z_r = 0.0f;
            params->_z_i = 0.0f;

            // calculate next pixel
            for (params->_n=0u; params->_n < params->num_iterations; params->_n++) {

                // square z
                params->_z_r_tmp = params->_z_r;
                params->_z_r = params->_z_r*params->_z_r - params->_z_i*params->_z_i;
                params->_z_i = 2.0f*params->_z_r_tmp*params->_z_i;

                // add
                params->_z_r += params->_col * 2.0f / params->cols - 1.5f;
                params->_z_i += params->_row * 2.0f / params->rows - 1.0f;

            }

            // set pixel
            params->img_part[params->_p-params->_p_begin] = sqrt(params->_z_r*params->_z_r + params->_z_i*params->_z_i) < 2.0f ? '#' : '.';

        } else {

            // insert newline
            params->img_part[params->_p-params->_p_begin] = '\n';

        }

    }

    #ifdef MEASURE_TIMING
    time_threads[params->thread_number] = get_timestamp(ts_begin_thread);
    #endif

    return NULL;

}

int main() {

    #ifdef MEASURE_TIMING
    ts_begin = get_timestamp();
    #endif

    // get amount of cores
    const auto NUM_CORES_STRING = getenv("MAX_CPUS");
    NUM_CORES = NUM_CORES_STRING == nullptr ? 1 : strtoul(NUM_CORES_STRING, NULL, 10);
    fprintf(stderr, "Working with %lu threads\n", NUM_CORES);

    // read stdin
    uint rows, cols, max_iterations;
    (void)! scanf("%u", &rows);
    (void)! scanf("%u", &cols);
    (void)! scanf("%u", &max_iterations);

    #ifdef MEASURE_TIMING
    time_preparation = get_timestamp(ts_begin);
    ts_calculation = get_timestamp();
    time_threads = new clock_t[NUM_CORES];
    #endif

    // calculate mandelbrot set
    auto *threads = new pthread_t[NUM_CORES];
    auto params_arr = new mandelbrot_params_t[NUM_CORES];
    for (uint i = 0u; i < NUM_CORES; i++) {

        params_arr[i].thread_number = i;
        params_arr[i].rows = rows;
        params_arr[i].cols = cols;
        params_arr[i].num_iterations = max_iterations;
        pthread_create(&threads[i], NULL, work, &params_arr[i]);

    }

    // wait for them to finish
    for (uint i = 0u; i < NUM_CORES; i++) {

        pthread_join(threads[i], NULL);

        // print result
        fwrite(params_arr[i].img_part, 1, (i+1)*rows*(cols+1)/NUM_CORES-i*rows*(cols+1)/NUM_CORES, stdout);

    }

    #ifdef MEASURE_TIMING
    time_calculation = get_timestamp(ts_calculation);
    ts_calculation = get_timestamp();
    time_full = get_timestamp(ts_begin);

    fprintf(stderr, "Time full: %.3fms\n", time_full/1.0e4);
    fprintf(stderr, "Time preparation: %.3fms (%.2f%%)\n", time_preparation/1.0e4, time_preparation*100.0/time_full);
    fprintf(stderr, "Time mandelbrot: %.3fms (%.2f%%)\n", time_calculation/1.0e4, time_calculation*100.0/time_full);

    for (uint i = 0u; i < NUM_CORES; i++) fprintf(stderr, "Time thread %u: %.3fms\n", i, time_threads[i]/1.0e4);
    #endif

    return 0;

}


