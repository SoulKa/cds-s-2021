#include <thread>
#include <cstring>
#include <atomic>

#include <math.h>

#ifdef MEASURE_TIMING
#include "common.h"
#endif

using namespace std;

// TYPEDEFS
typedef unsigned int uint;

typedef struct mandelbrot_params_t {
    uint8_t padding_front[64];
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
    uint8_t padding_back[64];
};

// GLOBAL VARS
unsigned long NUM_CORES;

#ifdef MEASURE_TIMING
int64_t ts_begin;
int64_t ts_calculation;

int64_t time_full;
int64_t time_preparation;
int64_t time_calculation;

int64_t *time_threads;
#endif

// FUNCTIONS

void work( mandelbrot_params_t *params )
{

    #ifdef MEASURE_TIMING
    auto ts_begin_thread = get_timestamp();
    #endif

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

}

int main() {

    #ifdef MEASURE_TIMING
    ts_begin = get_timestamp();
    #endif

    // get amount of cores
    const auto NUM_CORES_STRING = getenv("MAX_CPUS");
    NUM_CORES = NUM_CORES_STRING == nullptr ? 1 : stoul(NUM_CORES_STRING);

    // read stdin
    uint rows, cols, max_iterations;
    (void)! scanf("%u", &rows);
    (void)! scanf("%u", &cols);
    (void)! scanf("%u", &max_iterations);

    #ifdef MEASURE_TIMING
    time_preparation = get_timestamp(ts_begin);
    ts_calculation = get_timestamp();
    time_threads = new int64_t[NUM_CORES];
    #endif

    // calculate mandelbrot set
    thread *threads = new thread[NUM_CORES];
    auto params_arr = new mandelbrot_params_t[NUM_CORES];
    for (uint i = 0u; i < NUM_CORES; i++) {

        params_arr[i].thread_number = i;
        params_arr[i].rows = rows;
        params_arr[i].cols = cols;
        params_arr[i].num_iterations = max_iterations;
        threads[i] = thread(work, &params_arr[i]);

    }

    // wait for them to finish
    for (uint i = 0u; i < NUM_CORES; i++) {

        threads[i].join();

        // print result
        fwrite(params_arr[i].img_part, 1, (i+1)*rows*(cols+1)/NUM_CORES-i*rows*(cols+1)/NUM_CORES, stdout);

    }

    #ifdef MEASURE_TIMING
    time_calculation = get_timestamp(ts_calculation);
    ts_calculation = get_timestamp();
    time_full = get_timestamp(ts_begin);

    fprintf(stderr, "Time full: %.3fms\n", time_full/1.0e6);
    fprintf(stderr, "Time preparation: %.3fms (%.2f%%)\n", time_preparation/1.0e6, time_preparation*100.0/time_full);
    fprintf(stderr, "Time mandelbrot: %.3fms (%.2f%%)\n", time_calculation/1.0e6, time_calculation*100.0/time_full);

    for (uint i = 0u; i < NUM_CORES; i++) fprintf(stderr, "Time thread %u: %.3fms\n", i, time_threads[i]/1.0e6);
    #endif

    return 0;

}


