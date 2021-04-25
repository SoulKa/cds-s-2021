#include <thread>

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
#endif

// FUNCTIONS

void work( uint thread_number, uint rows, uint cols, uint num_iterations, char *img )
{

    uint n, row, col;
    float z_r, z_i, z_r_tmp;

    // iterate over all pixel that this thread has to calculate
    for (uint p = thread_number; p < rows*(cols+1u); p += NUM_CORES) {
    //for (uint p = p = thread_number*rows*(cols+1)/NUM_CORES; p < (thread_number+1)*rows*(cols+1)/NUM_CORES; p++) {

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
            img[row*(cols+1u)+col] = sqrt(z_r*z_r + z_i*z_i) < 2.0f ? '#' : '.';

        } else {
            img[row*(cols+1u)+col] = '\n';
        }
        

    }

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
    scanf("%u", &rows);
    scanf("%u", &cols);
    scanf("%u", &max_iterations);

    // create image
    const auto img_size = rows*(cols+1u);
    char *img = new char[img_size+1u];
    img[img_size] = '\0';

    #ifdef MEASURE_TIMING
    time_preparation = get_timestamp(ts_begin);
    ts_calculation = get_timestamp();
    #endif

    // calculate mandelbrot set
    thread *threads = new thread[NUM_CORES];
    for (uint i = 0u; i < NUM_CORES; i++) threads[i] = thread(work, i, rows, cols, max_iterations, img );

    // wait for them to finish
    for (uint i = 0u; i < NUM_CORES; i++) threads[i].join();

    #ifdef MEASURE_TIMING
    time_calculation = get_timestamp(ts_calculation);
    ts_calculation = get_timestamp();
    #endif

    // print result
    fwrite(img, 1, img_size, stdout);

    #ifdef MEASURE_TIMING
    time_full = get_timestamp(ts_begin);

    fprintf(stderr, "Time full: %.3fms\n", time_full/1.0e6);
    fprintf(stderr, "Time preparation: %.3fms (%.2f%%)\n", time_preparation/1.0e6, time_preparation*100.0/time_full);
    fprintf(stderr, "Time mandelbrot: %.3fms (%.2f%%)\n", time_calculation/1.0e6, time_calculation*100.0/time_full);
    #endif

    return 0;

}


