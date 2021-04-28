#include <complex>
#include <atomic>
#include <queue>

#include <pthread.h>
#include <sched.h>
#include <string.h>
#include <limits.h>
#include <unistd.h>

using namespace std;


// TYPEDEFS
#define CACHELINE_SIZE 64lu

struct pixel_value {
    uint32_t pixel_number = UINT32_MAX;
    char pixel_value;
};

struct alignas(CACHELINE_SIZE) mandelbrot_globals_t {
    uint8_t num_threads;
    uint32_t rows; 
    uint32_t cols;
    uint32_t img_size;
    uint32_t num_iterations;
    bool done = false;
    char *img;
    uint8_t padding_back[CACHELINE_SIZE];
};

struct alignas(CACHELINE_SIZE) mandelbrot_params_t {
    uint32_t input = UINT32_MAX;    // The next pixel to work on
    uint8_t padding1[CACHELINE_SIZE-sizeof(uint32_t)];
    pixel_value output;             // A finished pixel
    uint8_t padding2[CACHELINE_SIZE-sizeof(pixel_value)];
    uint8_t thread_number = 0u;
    uint8_t padding3[CACHELINE_SIZE-sizeof(uint8_t)];
};

struct alignas(CACHELINE_SIZE) mandelbrot_vars_t {
    uint32_t p;
    uint32_t r;
    uint32_t c;
    uint32_t n;
    complex<float> z;
    uint8_t padding_back[CACHELINE_SIZE-4*sizeof(uint32_t)-sizeof(complex<float>)];
};


// GLOBALS
mandelbrot_globals_t g;

// FUNCTIONS

bool set_on_cpu( const int &cpu )
{

    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(cpu, &cpuset);
    if (pthread_setaffinity_np(pthread_self(), sizeof(cpuset), &cpuset) != 0) {
        fprintf(stderr, "Could not set CPU affinity to CPU %d for handle %lu!\n", cpu, pthread_self());
        return false;
    } else {
        fprintf(stderr, "Assigned a thread to CPU %d\n", sched_getcpu());
        return true;
    }

}

void *work( void *params_uncasted )
{

    auto p = (mandelbrot_params_t *)params_uncasted;

    // set CPU affinity
    set_on_cpu(p->thread_number);

    mandelbrot_vars_t v;

    // work loop
    while ( true ) {

        // check for new work
        while (p->input == UINT32_MAX) {
            if (g.done) goto lbl_end;
            usleep(1);
        }
        v.p = p->input;
        p->input = UINT32_MAX;
        if (v.p >= g.img_size) goto lbl_end;

        // prepare vars
        v.z = 0;
        v.n = 0u;
        v.r = v.p / g.cols;
        v.c = v.p % g.cols;
        //fprintf(stderr, "Working on pixel %u (%u, %u)\n", v.p, v.r, v.c);
        
        // calculate pixel
        while (abs(v.z) < 2.0 && ++v.n < g.num_iterations) {
            v.z = v.z*v.z + complex<float>(
                    (float)v.c * 2 / g.cols - 1.5,
                    (float)v.r * 2 / g.rows - 1
                );
        }
        
        // set pixel
        while (p->output.pixel_number != UINT32_MAX) usleep(1);
        p->output.pixel_value = (v.n == g.num_iterations) ? '#' : '.';
        p->output.pixel_number = v.p;
        //fprintf(stderr, "Finished pixel %u\n", v.p);

    }

    // done
    lbl_end:
    fprintf(stderr, "Thread %u finished!\n", p->thread_number);
    return nullptr;

}

void *collect_output( void *thread_params_uncasted )
{

    // let the io threads be on the last CPU
    set_on_cpu(g.num_threads-1);

    auto params_arr = (mandelbrot_params_t*) thread_params_uncasted;

    uint32_t pixels_processed = 0u;
    while (pixels_processed < g.img_size) {

        for (uint8_t i = 0u; i < g.num_threads; i++) {

            // check for new pixels
            if (params_arr[i].output.pixel_number != UINT32_MAX) {

                // read pixel result from worker
                g.img[params_arr[i].output.pixel_number] = params_arr[i].output.pixel_value;
                //fprintf(stderr, "Received pixel: %u\n", params_arr[i].output.pixel_number);
                params_arr[i].output.pixel_number = UINT32_MAX;
                
                pixels_processed++;

            }

        }

    }

    return nullptr;

}

int main() {

    // let the main thread be on the first CPU
    if (!set_on_cpu(0)) return 1;

    // get amount of cores
    const char *NUM_CORES_STRING = getenv("MAX_CPUS");
    g.num_threads = NUM_CORES_STRING == NULL ? 1 : strtoul(NUM_CORES_STRING, NULL, 10);
    fprintf(stderr, "Working with %u threads\n", g.num_threads);

    // read parameters
    (void)! scanf("%u", &g.rows);
    (void)! scanf("%u", &g.cols);
    (void)! scanf("%u", &g.num_iterations);

    // create image
    g.img_size = g.rows*g.cols;
    g.img = new char[g.img_size];

    // calculate mandelbrot set
    auto *threads = new pthread_t[g.num_threads];
    auto params_arr = new mandelbrot_params_t[g.num_threads];
    u_char i;
    for (i = 0u; i < g.num_threads; i++) {

        // set parameters for thread
        params_arr[i].thread_number = i;
        params_arr[i].input = i;
        
        // create thread
        pthread_create(&threads[i], NULL, work, &params_arr[i]);

    }

    // create output collector thread
    pthread_t output_thread;
    pthread_create(&output_thread, NULL, collect_output, params_arr);

    // feed the little threads
    uint32_t p = g.num_threads;
    while ( p < g.img_size ) {

        for (i = 0u; i < g.num_threads && p < g.img_size; i++) {

            // provide new input for worker
            if (params_arr[i].input == UINT32_MAX) {
                //fprintf(stderr, "Next pixel: %u\n", p);
                params_arr[i].input = p++;
            }

        }

    }
    for (i = 0u; i < g.num_threads; i++) while (params_arr[i].input != UINT32_MAX) usleep(1);
    g.done = true;

    // wait for them to finish
    fprintf(stderr, "All input distributed. Waiting for threads to finish...\n");
    uint8_t joined_threads = 0u;
    while (joined_threads < g.num_threads) {

        for (i = 0u; i < g.num_threads; i++) {

            // skip already joined threads
            if (threads[i] == UINT64_MAX) continue;

            // check if joinable
            if (pthread_tryjoin_np(threads[i], NULL) == 0) {
                joined_threads++;
                threads[i] = UINT64_MAX;
                fprintf(stderr, "Joined thread %u\n", i);
            }

        }

    }

    // wait for the output worker to collect the last results
    pthread_join(output_thread, NULL);

    // write result
    fprintf(stderr, "Printing result...\n");
    for (auto img_ptr = g.img; img_ptr < g.img+g.img_size; img_ptr += g.cols) {
        fwrite(img_ptr, sizeof(g.img[0u]), g.cols, stdout);
        fputc('\n', stdout);
    }

    return 0;

}


