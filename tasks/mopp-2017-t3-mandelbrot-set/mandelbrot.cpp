#include <complex>
#include <iostream>
#include <thread>
#include <mutex>

#include <assert.h>

using namespace std;

// TYPEDEFS
typedef struct {
    const unsigned int MAX_ROW;
    const unsigned int MAX_COL;
    const unsigned int MAX_N;
    const float COL_ITER;
    const float ROW_ITER;
    char *IMAGE;
} s_constants_t;

// GLOBAL VARS
mutex pixel_mutex;
unsigned int current_pixel = 0;

// FUNCTIONS

void calculate( unsigned int r, unsigned int c, s_constants_t *constants ) {

    complex<float> z = 0.0f;
    unsigned int n = 0;

    while (abs(z) < 2.0f && ++n < constants->MAX_N) {
        z = z*z + complex<float>(
            c * 2.0f / constants->MAX_COL - 1.5f,
            r * 2.0f / constants->MAX_ROW - 1.0f
        );
    }

    // set pixel
    constants->IMAGE[r*constants->MAX_COL+c] = (n == constants->MAX_N ? '#' : '.');

}

void work( s_constants_t *constants )
{

    const unsigned int MAX_PIXEL = constants->MAX_COL*constants->MAX_ROW;
    unsigned int p;

    while (true) {
        pixel_mutex.lock();
        p = current_pixel++;
        pixel_mutex.unlock();

        // check if done
        if (p >= MAX_PIXEL) break;

        // calculate next pixel
        calculate(p / constants->MAX_COL, p % constants->MAX_COL, constants );
    }

}

int main() {

    // get amount of cores
    const auto NUM_CORES = getenv("MAX_CPUS") == nullptr ? thread::hardware_concurrency() : atoi(getenv("MAX_CPUS"));
    assert((NUM_CORES > 0 && NUM_CORES <= 56) && "Could not get the number of cores!");
    cerr << "Working with " << NUM_CORES << " cores" << endl;

    // read stdin
    unsigned int max_row, max_col, max_n;
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
    for (unsigned int i = 0; i < NUM_CORES; i++) {
        threads[i] = thread(work, &constants );
    }

    // wait for them to finish
    for (unsigned int i = 0; i < NUM_CORES; i++) {
        threads[i].join();
    }

    // print result
    for (unsigned int r = 0; r < max_row; ++r) {
        for (unsigned int c = 0; c < max_col; ++c) cout << img[r*max_col+c];
        cout << '\n';
    }    
}


