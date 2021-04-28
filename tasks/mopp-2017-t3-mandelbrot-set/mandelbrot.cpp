#include <complex>
#include <atomic>

#include <pthread.h>
#include <sched.h>
#include <string.h>

using namespace std;

typedef u_char byte;

struct mandelbrot_params_t {
    byte padding_front[64];
    uint thread_number;
    uint rows;
    uint cols;
    uint num_iterations;
	atomic<uint> *p_next_row;
    char *img;
    byte padding_back[64];
};

void *work( void *params_uncasted )
{

	auto p = (mandelbrot_params_t *)params_uncasted;

	// set CPU affinity
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(p->thread_number, &cpuset);
    if (pthread_setaffinity_np(pthread_self(), sizeof(cpuset), &cpuset) != 0) {
        fprintf(stderr, "Could not set CPU affinity for thread %u (handle %lu)!\n", p->thread_number, pthread_self());
    } else {
        fprintf(stderr, "Running thread %u on CPU %d\n", p->thread_number, sched_getcpu());
    }

	uint r, c;
	while ( (r = (*p->p_next_row)++) < p->rows ) {

		for (c = 0; c < p->cols; ++c){

			complex<float> z = 0;

			uint n = 0;
			while (abs(z) < 2.0 && ++n < p->num_iterations) {
				z = z*z + complex<float>(
					(float)c * 2 / p->cols - 1.5,
					(float)r * 2 / p->rows - 1
				);
			}
			
			p->img[r*(p->cols+1)+c] = (n == p->num_iterations ? '#' : '.');

		}

		p->img[r*(p->cols+1)+c] = '\n';

	}

	return nullptr;

}

int main(){

	// get amount of cores
    const char *NUM_CORES_STRING = getenv("MAX_CPUS");
    u_char NUM_CORES = NUM_CORES_STRING == NULL ? 1 : strtoul(NUM_CORES_STRING, NULL, 10);
    fprintf(stderr, "Working with %u threads\n", NUM_CORES);

	// read parameters
	uint rows, cols, max_iterations;
	(void)! scanf("%u", &rows);
    (void)! scanf("%u", &cols);
    (void)! scanf("%u", &max_iterations);

	// create image
	uint img_size = sizeof(char)*rows*(cols+1);
	char *img = (char*)malloc(img_size);

	// prepare shared variable
	atomic<uint> next_row(0u);

	// calculate mandelbrot set
    auto *threads = new pthread_t[NUM_CORES];
    auto params_arr = new mandelbrot_params_t[NUM_CORES];
    u_char i;
    for (i = 0u; i < NUM_CORES; i++) {

        params_arr[i].thread_number = i;
		params_arr[i].rows = rows;
		params_arr[i].cols = cols;
		params_arr[i].num_iterations = max_iterations;
		params_arr[i].img = img;
		params_arr[i].p_next_row = &next_row;
        
        if (i == NUM_CORES-1)
            work(&params_arr[i]);
        else
            pthread_create(&threads[i], NULL, work, &params_arr[i]);

    }

    // wait for them to finish
    for (i = 0u; i < NUM_CORES; i++) if (i != NUM_CORES-1) pthread_join(threads[i], NULL);

	// write result
	fwrite(img, sizeof(img[0]), img_size, stdout);

	return 0;

}


