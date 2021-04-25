#include <iostream>
#include <thread>

#include <assert.h>
#include <math.h>

using namespace std;

// TYPEDEFS

typedef struct {
    const uint MAX_ROW;
    const uint MAX_COL;
    const uint MAX_N;
    char *IMAGE;
} s_constants_t;

typedef unsigned int uint;



// GLOBAL VARS
uint NUM_CORES;


// FUNCTIONS

void work( uint thread_number, s_constants_t *constants )
{

    uint p = thread_number*constants->MAX_ROW*(constants->MAX_COL+1)/NUM_CORES;
    uint p_end = (thread_number+1)*constants->MAX_ROW*(constants->MAX_COL+1)/NUM_CORES;
    uint n, row, col;
    float z_r, z_i, z_r_tmp;

    // iterate over all pixel that this thread has to calculate
    for (; p < p_end; p++) {

        
        row = p / (constants->MAX_COL+1);
        col = p % (constants->MAX_COL+1);

        if (col < constants->MAX_COL ) {

            // prepare variables for next iteration
            z_r = 0.0;
            z_i = 0.0;

            // calculate next pixel
            for (n=1u; n < constants->MAX_N; n++) {

                // square z
                z_r_tmp = z_r;
                z_r = z_r*z_r - z_i*z_i;
                z_i = 2*z_r_tmp*z_i;

                // add
                z_r += col * 2.0f / constants->MAX_COL - 1.5f;
                z_i += row * 2.0f / constants->MAX_ROW - 1.0f;

            }

            // set pixel
            constants->IMAGE[row*(constants->MAX_COL+1)+col] = sqrt(z_r*z_r + z_i*z_i) < 2.0f ? '#' : '.';

        } else {
            constants->IMAGE[row*(constants->MAX_COL+1)+col] = '\n';
        }
        

    }

}

int main() {

    // get amount of cores
    NUM_CORES = getenv("MAX_CPUS") == nullptr ? thread::hardware_concurrency() : atoi(getenv("MAX_CPUS"));
    assert((NUM_CORES > 0 && NUM_CORES <= 56) && "Could not get the number of cores!");
    cerr << "Working with " << NUM_CORES << " cores" << endl;

    // read stdin
    uint max_row, max_col, max_n;
    cin >> max_row;
    cin >> max_col;
    cin >> max_n;

    // create image
    char *img = new char[max_row*(max_col+1)+1];
    img[max_row*(max_col+1)] = '\0';

    // CONSTANTS
    s_constants_t constants = {
        max_row,
        max_col,
        max_n,
        img
    };

    // calculate mandelbrot set
    thread *threads = new thread[NUM_CORES];
    for (uint i = 0; i < NUM_CORES; i++) threads[i] = thread(work, i, &constants );

    // wait for them to finish
    for (uint i = 0; i < NUM_CORES; i++) threads[i].join();

    // print result
    printf("%s", img);  
}


