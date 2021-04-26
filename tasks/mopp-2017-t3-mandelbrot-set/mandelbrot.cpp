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

void work( uint thread_number, uint rows, uint cols, uint num_iterations, char **img )
{

    #ifdef MEASURE_TIMING
    auto ts_begin_thread = get_timestamp();
    #endif

    uint p_begin = thread_number*rows*(cols+1u)/NUM_CORES;
    uint p_end = (thread_number+1u)*rows*(cols+1u)/NUM_CORES;

    uint n, row, col;
    float z_r, z_i, z_r_tmp;
    auto img_part = new char[p_end-p_begin+128u]+64u; // add 64byte before and after to make sure that no cache miss will happen
    //memset(img_part, 0, p_end-p_begin);

    // iterate over all pixel that this thread has to calculate
    //for (uint p = thread_number; p < rows*(cols+1u); p += NUM_CORES) {
    for (uint p = p_begin; p < p_end; p++) {

        row = p / (cols+1u);
        col = p % (cols+1u);

        if (col < cols) {

            // prepare variables for next iteration
            z_r = 0.0f;
            z_i = 0.0f;

            // calculate next pixel
            for (n=0u; n < num_iterations; n++) {

                // square z
                z_r_tmp = z_r;
                z_r = z_r*z_r - z_i*z_i;
                z_i = 2.0f*z_r_tmp*z_i;

                // add
                z_r += col * 2.0f / cols - 1.5f;
                z_i += row * 2.0f / rows - 1.0f;

            }

            // set pixel
            img_part[p-p_begin] = sqrt(z_r*z_r + z_i*z_i) < 2.0f ? '#' : '.';

        } else {

            // insert newline
            img_part[p-p_begin] = '\n';

        }

    }

    // copy image part into final image
    //memcpy(img+p_begin, img_part, p_end-p_begin);
    *img = img_part;

    #ifdef MEASURE_TIMING
    time_threads[thread_number] = get_timestamp(ts_begin_thread);
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

    // create image
    /*const auto img_size = rows*(cols+1u);
    char *img = new char[img_size+1u];
    img[img_size] = '\0';*/
    auto img_ptr_arr = new char *[NUM_CORES];

    #ifdef MEASURE_TIMING
    time_preparation = get_timestamp(ts_begin);
    ts_calculation = get_timestamp();
    time_threads = new int64_t[NUM_CORES];
    #endif

    // calculate mandelbrot set
    thread *threads = new thread[NUM_CORES];
    for (uint i = 0u; i < NUM_CORES; i++) {
        threads[i] = thread(work, i, rows, cols, max_iterations, &img_ptr_arr[i] );
    }

    // wait for them to finish
    for (uint i = 0u; i < NUM_CORES; i++) {

        threads[i].join();

        // print result
        fwrite(img_ptr_arr[i], 1, (i+1)*rows*(cols+1)/NUM_CORES-i*rows*(cols+1)/NUM_CORES, stdout);

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


