#include <complex>
#include <iostream>
#include <thread>
#include <mutex>
#include <atomic>

#include <assert.h>

using namespace std;

// TYPEDEFS

typedef struct {
    const uint MAX_ROW;
    const uint MAX_COL;
    const uint MAX_N;
    const float COL_ITER;
    const float ROW_ITER;
    char *IMAGE;
} s_constants_t;

typedef unsigned int uint;



// GLOBAL VARS

atomic<uint> current_pixel(0);
uint NUM_CORES;


// FUNCTIONS

void work( s_constants_t *constants )
{

    const uint MAX_PIXEL = constants->MAX_COL*constants->MAX_ROW;
    uint step_size = constants->MAX_COL;

    uint p, p_begin, p_end, row, col, n;
    complex<float> z;

    while (true) {

        // get next working set
        p_begin = (current_pixel += step_size);
        p_begin -= step_size;
        p_end = min(p_begin+step_size, MAX_PIXEL);

        // check if done
        if (p_begin >= MAX_PIXEL) break;

        for (p = p_begin; p < p_end; p++) {

            //if (p % 1000 == 0) cerr << p << endl;

            // prepare variables for next iteration
            row = p / constants->MAX_COL;
            col = p % constants->MAX_COL;
            z = 0.0f;
            n = 0u;

            // calculate next pixel
            while (abs(z) < 2.0f && ++n < constants->MAX_N) {
                z = z*z + complex<float>(
                    col * 2.0f / constants->MAX_COL - 1.5f,
                    row * 2.0f / constants->MAX_ROW - 1.0f
                );
            }

            // set pixel
            constants->IMAGE[row*constants->MAX_COL+col] = (n == constants->MAX_N ? '#' : '.');

        }

        // adjust stepsize
        if (p_end + NUM_CORES*4 > MAX_PIXEL) {
            step_size = 1;
        } else {
            step_size = max(uint(constants->MAX_COL / 2.0 * (1.0 - ((double)p_end/MAX_PIXEL))), 1u);
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
    char *img = new char[max_row*max_col];

    // CONSTANTS
    s_constants_t constants = {
        max_row,
        max_col,
        max_n,
        2.0f / max_col,
        2.0f / max_row,
        img
    };

    // calculate mandelbrot set
    thread *threads = new thread[NUM_CORES];
    for (uint i = 0; i < NUM_CORES; i++) {
        threads[i] = thread(work, &constants );
    }

    // wait for them to finish
    for (uint i = 0; i < NUM_CORES; i++) {
        threads[i].join();
    }

    // print result
    for (uint r = 0; r < max_row; ++r) {
        for (uint c = 0; c < max_col; ++c) cout << img[r*max_col+c];
        cout << '\n';
    }    
}


