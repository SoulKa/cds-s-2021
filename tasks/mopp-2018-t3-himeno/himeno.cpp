/********************************************************************

 This benchmark test program is measuring a cpu performance
 of floating point operation by a Poisson equation solver.

 If you have any question, please ask me via email.
 written by Ryutaro HIMENO, November 26, 2001.
 Version 3.0
 ----------------------------------------------
 Ryutaro Himeno, Dr. of Eng.
 Head of Computer Information Division,
 RIKEN (The Institute of Pysical and Chemical Research)
 Email : himeno@postman.riken.go.jp
 ---------------------------------------------------------------
 This program is to measure a computer performance in MFLOPS
 by using a kernel which appears in a linear solver of pressure
 Poisson eq. which appears in an incompressible Navier-Stokes solver.
 A point-Jacobi method is employed in this solver as this method can 
 be easyly vectrized and be parallelized.
 ------------------
 Finite-difference method, curvilinear coodinate system
 Vectorizable and parallelizable on each grid point
 No. of grid points : imax x jmax x kmax including boundaries
 ------------------
 A,B,C:coefficient matrix, wrk1: source term of Poisson equation
 wrk : working area, OMEGA : relaxation parameter
 BND:control variable for boundaries and objects ( = 0 or 1)
 P: pressure
********************************************************************/

#include "himeno.h"

#include <mutex>
#include <thread>
#include <atomic>
#include <chrono>

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

using namespace std;

// DEFINES
#define OMEGA 0.8
#define ONE_SIXTH 1.0/6.0


// GLOBAL VARS
uint NUM_CORES;
Matrix<FLOAT_TYPE_TO_USE> *p;
Matrix<FLOAT_TYPE_TO_USE> *wrk;
atomic<int> current_row(0);

#ifdef USE_FLOAT64
    #define GOSA_POINTER gosa_arr+i
#else
    mutex gosa_mutex;
    #define GOSA_POINTER &gosa
#endif

#if MEASURE_TIME
    int64_t ts_beginning;
    int64_t ts_jacobi_beginning;
    int64_t ts_temp;

    int64_t time_calculation = 0;
    int64_t time_copying = 0;
    int64_t time_preparation = 0;
    int64_t time_jacobi = 0;
    int64_t time_full = 0;

    int64_t *times_threads;
#endif

// FUNCTIONS

int main( int argc, char *argv[] ) {

    #ifdef MEASURE_TIME
        ts_beginning = get_timestamp();
    #endif

    uint num_iterations, num_rows, num_cols, num_deps;

    // get amount of cores
    NUM_CORES = atoi(getenv("MAX_CPUS"));
    fprintf(stderr, "Working with %u cores\n", NUM_CORES);

    if (argc == 5) {
        num_rows = stoul(argv[1]);
        num_cols = stoul(argv[2]);
        num_deps = stoul(argv[3]);
        num_iterations = stoul(argv[4]);
    } else {
        (void)! scanf("%u", &num_rows);
        (void)! scanf("%u", &num_cols);
        (void)! scanf("%u", &num_deps);
        (void)! scanf("%u", &num_iterations);
    }

    fprintf(stderr, "Matrix size is %ux%ux%u with %u iterations\n", num_rows, num_cols, num_deps, num_iterations);

    // create matrices
    p = new Matrix<FLOAT_TYPE_TO_USE>(num_rows-2, num_cols-2, num_deps-2, NUM_CORES);
    wrk =  new Matrix<FLOAT_TYPE_TO_USE>(num_rows-2, num_cols-2, num_deps-2, NUM_CORES);

    // initialize matrices
    p->set_init();
    Matrix<FLOAT_TYPE_TO_USE>::copy(p, wrk);

    #ifdef MEASURE_TIME
        time_preparation = get_timestamp(ts_beginning);
        ts_jacobi_beginning = get_timestamp();
        times_threads = new int64_t[NUM_CORES];
        fill_n(times_threads, NUM_CORES, 0);
    #endif

    // print result
    printf("%.6f\n", jacobi(num_iterations));

    #ifdef MEASURE_TIME
        time_jacobi = get_timestamp(ts_jacobi_beginning);
        time_full = get_timestamp(ts_beginning);

        fprintf(stderr, "Time full: %.3fms\n", time_full/1.0e6);
        fprintf(stderr, "Time preparation: %.3fms (%.2f%%)\n", time_preparation/1.0e6, time_preparation*100.0/time_full);
        fprintf(stderr, "Time jacobi: %.3fms (%.2f%%)\n", time_jacobi/1.0e6, time_jacobi*100.0/time_full);
        /*fprintf(stderr, "  |--> Time calculated: %.3fms (%.2f%%)\n", time_calculation/1.0e6, time_calculation*100.0/time_jacobi);
        fprintf(stderr, "  +--> Time copied: %.3fms (%.2f%%)\n", time_copying/1.0e6, time_copying*100.0/time_jacobi);*/

        fprintf(stderr, "\n");
        for (uint i = 0; i < NUM_CORES; i++) fprintf(stderr, "Time thread %u: %.3fms\n", i, times_threads[i]/1.0e6);
        fprintf(stderr, "\n");
        delete[] times_threads;
    #endif

    return 0;

}

void calculate_part( uint thread_number, FLOAT_TYPE_TO_USE *gosa ) {

    #ifdef MEASURE_TIME
        const auto now = get_timestamp();
    #endif

    // vars
    FLOAT_TYPE_TO_USE value;
    FLOAT_TYPE_TO_USE *ptr_p_data;
    FLOAT_TYPE_TO_USE *ptr_wrk_data;
    int r, c, d;

    // iterate over the volume
    while ((r = current_row++) < p->m_uiRows) {

        // adjust pointers to fit the current row
        ptr_p_data = p->m_pData + r*p->m_uiRowMemoryOffset;
        ptr_wrk_data = wrk->m_pData + r*p->m_uiRowMemoryOffset;

        for (c = 0; c < p->m_uiCols; c++) {
            for (d = 0; d < p->m_uiDeps; d++) {

                // sum up the neighboring values
                value = (
                      p->get(r+1,c,d) + p->get(r,c+1,d) + p->get(r,c,d+1)
                    + p->get(r-1,c,d) + p->get(r,c-1,d) + p->get(r,c,d-1)
                ) / 6.0 - (*ptr_p_data);

                // check if it is last iteration
                if (gosa == nullptr) {
                    (*ptr_wrk_data) = (*ptr_p_data) + OMEGA*value;
                } else {
                    #ifndef USE_FLOAT64
                        gosa_mutex.lock();
                    #endif
                    (*gosa) += value*value;
                    #ifndef USE_FLOAT64
                        gosa_mutex.unlock();
                    #endif
                }

                // update pointers
                ptr_p_data++;
                ptr_wrk_data++;

            }
        }
    }

    #ifdef MEASURE_TIME
        times_threads[thread_number] += get_timestamp(now);
    #endif
    
}

FLOAT_TYPE_TO_USE jacobi( uint num_iterations ) {

    // for the final (combined) result
    FLOAT_TYPE_TO_USE gosa = 0.0f;
    Matrix<FLOAT_TYPE_TO_USE> *p_mat_tmp;
    const auto NUM_CORES_MINUS_ONE = NUM_CORES-1;

    // create thread array
    auto thread_arr = new thread[NUM_CORES_MINUS_ONE];

    // the partial results
    #ifdef USE_FLOAT64
        auto gosa_arr = new FLOAT_TYPE_TO_USE[NUM_CORES];
        fill_n(gosa_arr, NUM_CORES, 0.0f);
    #endif

    // start to calculate
    uint i;
    for (uint n = 0; n < num_iterations; n++) {

        #ifdef MEASURE_TIME
            ts_temp = get_timestamp();
        #endif

        // calculate in parallel
        for (i=0; i<NUM_CORES_MINUS_ONE; i++) thread_arr[i] = thread(calculate_part, i, n == num_iterations-1 ? GOSA_POINTER : nullptr);
        calculate_part(i, n == num_iterations-1 ? GOSA_POINTER : nullptr);
        for (i=0; i<NUM_CORES_MINUS_ONE; i++) thread_arr[i].join();
        current_row = 0;

        #ifdef MEASURE_TIME
            time_calculation += get_timestamp(ts_temp);
        #endif

        // swap matrices (no copy needed)
        p_mat_tmp = p;
        p = wrk;
        wrk = p_mat_tmp;
        
    }

    // sum up partial gosa
    #ifdef USE_FLOAT64
        for (uint i=0; i<NUM_CORES; i++) gosa += gosa_arr[i];
        delete[] gosa_arr;
    #endif

    // free ressources
    delete[] thread_arr;

    // done
    return gosa;
}

