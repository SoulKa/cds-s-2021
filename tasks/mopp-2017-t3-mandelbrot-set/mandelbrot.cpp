#include <complex>
#include <atomic>
#include <queue>
#include <chrono>
#include <thread>
#include <vector>
#include <atomic>

#include <pthread.h>
#include <sched.h>
#include <string.h>
#include <limits.h>
#include <unistd.h>


// TYPEDEFS
#define CACHELINE_SIZE 64lu
#define INPUT_BUFFER_SIZE 16u
#define OUTPUT_BUFFER_SIZE 8u

struct alignas(8) pixel_value {
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
    uint8_t padding_back[CACHELINE_SIZE-sizeof(uint8_t)-4*sizeof(uint32_t)-sizeof(bool)-sizeof(char*)];
};

struct alignas(CACHELINE_SIZE) mandelbrot_params_t {
    uint32_t input[INPUT_BUFFER_SIZE];
    pixel_value output[OUTPUT_BUFFER_SIZE];
    uint8_t thread_number;
    mandelbrot_params_t() {
        memset(input, -1, INPUT_BUFFER_SIZE*sizeof(input[0]));
    }
};

struct alignas(CACHELINE_SIZE) mandelbrot_vars_t {
    uint8_t input_queue_begin = 0u;
    uint8_t output_queue_end = 0u;
    uint32_t p;
    uint32_t r;
    uint32_t c;
    uint32_t n;
    std::complex<float> z;
    uint8_t padding[CACHELINE_SIZE-4*sizeof(uint32_t)-sizeof(std::complex<float>)];
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
        while ( p->input[v.input_queue_begin] == UINT32_MAX ) {
            if (g.done) goto lbl_end;
            std::this_thread::sleep_for(std::chrono::nanoseconds(1));
        }
        v.p = p->input[v.input_queue_begin];
        p->input[v.input_queue_begin] = UINT32_MAX;
        v.input_queue_begin = (v.input_queue_begin + 1) % INPUT_BUFFER_SIZE;

        // prepare vars
        v.z = 0;
        v.n = 0u;
        v.r = v.p / g.cols;
        v.c = v.p % g.cols;
        //fprintf(stderr, "Working on pixel %u (%u, %u)\n", v.p, v.r, v.c);
        
        // calculate pixel
        while (abs(v.z) < 2.0f && ++v.n < g.num_iterations) {
            v.z = v.z*v.z;
            v.z.real( v.z.real() + ((float)v.c * 2.0f / g.cols - 1.5f) );
            v.z.imag( v.z.imag() + ((float)v.r * 2.0f / g.rows - 1.0f) );
        }
        
        // set pixel
        while ( p->output[v.output_queue_end].pixel_number != UINT32_MAX ) std::this_thread::sleep_for(std::chrono::nanoseconds(1));
        p->output[v.output_queue_end].pixel_value = (v.n == g.num_iterations) ? '#' : '.';
        p->output[v.output_queue_end].pixel_number = v.p;
        v.output_queue_end = (v.output_queue_end + 1) % OUTPUT_BUFFER_SIZE;
        //fprintf(stderr, "Finished pixel %u\n", v.p);

    }

    // done
    lbl_end:
    fprintf(stderr, "Thread %u finished!\n", p->thread_number);
    return nullptr;

}

void *collect_output( void *thread_params_uncasted )
{

    // last CPU
    //set_on_cpu(g.num_threads-1);

    auto params_arr = (mandelbrot_params_t*) thread_params_uncasted;

    uint32_t pixels_processed = 0u;
    auto output_queues_begin = new uint8_t[g.num_threads];
    for (uint8_t i = 0u; i < g.num_threads; i++) output_queues_begin[i] = 0u;
    while (pixels_processed < g.img_size) {

        for (uint8_t i = 0u; i < g.num_threads; i++) {

            // check for new pixels
            while (params_arr[i].output[output_queues_begin[i]].pixel_number != UINT32_MAX) {

                // read pixel result from worker
                g.img[params_arr[i].output[output_queues_begin[i]].pixel_number] = params_arr[i].output[output_queues_begin[i]].pixel_value;
                //fprintf(stderr, "Received pixel: %u\n", params_arr[i].output[output_queues_begin[i]].pixel_number);
                params_arr[i].output[output_queues_begin[i]].pixel_number = UINT32_MAX;
                output_queues_begin[i] = (output_queues_begin[i] + 1) % OUTPUT_BUFFER_SIZE;
                
                pixels_processed++;

            }

        }

        //std::this_thread::sleep_for(std::chrono::nanoseconds(1));

    }

    fprintf(stderr, "Finished collecting the outputs\n");
    return nullptr;

}

void *provide_input( void *thread_params_uncasted )
{

    // first CPU
    //set_on_cpu(0);

    auto params_arr = (mandelbrot_params_t*) thread_params_uncasted;

    // feed the little threads
    uint8_t i;
    uint32_t p = 0;
    auto input_queues_end = new uint8_t[g.num_threads];
    while ( true ) {

        for (i = 0u; i < g.num_threads && p < g.img_size; i++) {

            // provide new input for worker
            while (params_arr[i].input[input_queues_end[i]] == UINT32_MAX) {
                //fprintf(stderr, "Next pixel: %u\n", p);
                params_arr[i].input[input_queues_end[i]] = p++;
                input_queues_end[i] = (input_queues_end[i] + 1) % INPUT_BUFFER_SIZE;
                if (p == g.img_size) goto lbl_end;
            }

        }

        //std::this_thread::sleep_for(std::chrono::nanoseconds(1));

    }

    lbl_end:
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
    
    // create input feeder thread
    pthread_t input_thread, output_thread;
    auto params_arr = new mandelbrot_params_t[g.num_threads];
    pthread_create(&input_thread, NULL, provide_input, params_arr);

    // let workers calculate
    auto *threads = new pthread_t[g.num_threads];
    u_char i;
    for (i = 0u; i < g.num_threads; i++) {

        // set parameters for thread
        params_arr[i].thread_number = i;
        
        // create thread
        pthread_create(&threads[i], NULL, work, params_arr+i);

    }

    // create output collector and wait
    pthread_create(&output_thread, NULL, collect_output, params_arr);
    pthread_join(input_thread, NULL);
    pthread_join(output_thread, NULL);
    g.done = true;

    // wait for them to finish
    fprintf(stderr, "All input distributed. Waiting for worker threads to finish...\n");
    for (i = 0u; i < g.num_threads; i++) {
        pthread_join(threads[i], NULL);
        fprintf(stderr, "Joined thread %u\n", i);
    }

    // write result
    fprintf(stderr, "Printing result...\n");
    for (auto img_ptr = g.img; img_ptr < g.img+g.img_size; img_ptr += g.cols) {
        fwrite(img_ptr, sizeof(g.img[0u]), g.cols, stdout);
        fputc('\n', stdout);
    }

    return 0;

}


