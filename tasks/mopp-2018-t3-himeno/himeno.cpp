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
 wrk2 : working area, OMEGA : relaxation parameter
 BND:control variable for boundaries and objects ( = 0 or 1)
 P: pressure
********************************************************************/

#include "himeno.h"

#include <mutex>
#include <thread>
#include <iostream>
#include <atomic>
#include <chrono>

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

using namespace std;

#define MEASURE_TIME false

// CONSTANTS
const double OMEGA = 0.8;

// GLOBAL VARS
uint NUM_CORES;

#if MEASURE_TIME
    int64_t ts_beginning;
    int64_t ts_jacobi_beginning;
    int64_t ts_temp;

    int64_t time_calculation = 0;
    int64_t time_copying = 0;
    int64_t time_preparation = 0;
    int64_t time_jacobi = 0;
    int64_t time_full = 0;
#endif

// FUNCTIONS

int main() {

    #if MEASURE_TIME
        ts_beginning = get_timestamp();
    #endif

    uint nn, mimax, mjmax, mkmax, msize[3];

    // get amount of cores
    NUM_CORES = getenv("MAX_CPUS") == nullptr ? thread::hardware_concurrency() : atoi(getenv("MAX_CPUS"));
    assert((NUM_CORES > 0 && NUM_CORES <= 56) && "Could not get the number of cores!");
    cerr << "Working with " << NUM_CORES << " cores" << endl;

    cin >> msize[0];
    cin >> msize[1];
    cin >> msize[2];
    cin >> nn;
    
    mimax = msize[0];
    mjmax = msize[1];
    mkmax = msize[2];

    // create matrices
    matrix_set_t matrices = {
        mat_float64_t(4, mimax, mjmax, mkmax, NUM_CORES),   // a
        mat_float64_t(3, mimax, mjmax, mkmax, NUM_CORES),   // b
        mat_float64_t(3, mimax, mjmax, mkmax, NUM_CORES),   // c
        mat_float64_t(1, mimax, mjmax, mkmax, NUM_CORES),   // p
        mat_float64_t(1, mimax, mjmax, mkmax, NUM_CORES),   // bnd
        mat_float64_t(1, mimax, mjmax, mkmax, NUM_CORES),   // wrk1
        mat_float64_t(1, mimax, mjmax, mkmax, NUM_CORES)    // wrk2
    };

    // initialize matrices
    matrices.p.set_init();
    matrices.bnd.fill(1.0, 0);
    matrices.wrk1.fill(0.0, 0);
    mat_float64_t::copy(&matrices.p, &matrices.wrk2);

    matrices.a.fill_partial(1.0, 0, 3);
    matrices.a.fill(1.0/6.0, 3);
    matrices.b.fill(0.0);
    matrices.c.fill(1.0);

    #if MEASURE_TIME
        time_preparation = get_timestamp(ts_beginning);
        ts_jacobi_beginning = get_timestamp();
    #endif

    // print result
    printf("%.6f\n", jacobi(nn, &matrices));

    #if MEASURE_TIME
        time_jacobi = get_timestamp(ts_jacobi_beginning);
        time_full = get_timestamp(ts_beginning);
        fprintf(stderr, "Time full: %.3fms\n", time_full/1.0e6);
        fprintf(stderr, "Time preparation: %.3fms (%.2f%%)\n", time_preparation/1.0e6, time_preparation*100.0/time_full);
        fprintf(stderr, "Time jacobi: %.3fms (%.2f%%)\n", time_jacobi/1.0e6, time_jacobi*100.0/time_full);
        fprintf(stderr, "  |--> Time calculated: %.3fms (%.2f%%)\n", time_calculation/1.0e6, time_calculation*100.0/time_jacobi);
        fprintf(stderr, "  +--> Time copied: %.3fms (%.2f%%)\n", time_copying/1.0e6, time_copying*100.0/time_jacobi);
    #endif

    return 0;

}

void calculate_at( double *gosa, matrix_set_t *matrices, uint i, uint j, uint k ) {

    double s0 = matrices->a.at(0,i,j,k) * matrices->p.at(0,i+1,j,k)
        + matrices->a.at(1,i,j,k) * matrices->p.at(0,i,j+1,k)
        + matrices->a.at(2,i,j,k) * matrices->p.at(0,i,j,k+1)
        + matrices->b.at(0,i,j,k) * (
            matrices->p.at(0,i+1,j+1,k)
            - matrices->p.at(0,i+1,j-1,k)
            - matrices->p.at(0,i-1,j+1,k)
            + matrices->p.at(0,i-1,j-1,k)
        )
        + matrices->b.at(1,i,j,k) * (
            matrices->p.at(0,i,j+1,k+1)
            - matrices->p.at(0,i,j-1,k+1)
            - matrices->p.at(0,i,j+1,k-1)
            + matrices->p.at(0,i,j-1,k-1)
        )
        + matrices->b.at(2,i,j,k) * (
            matrices->p.at(0,i+1,j,k+1)
            - matrices->p.at(0,i-1,j,k+1)
            - matrices->p.at(0,i+1,j,k-1)
            + matrices->p.at(0,i-1,j,k-1)
        )
        + matrices->c.at(0,i,j,k) * matrices->p.at(0,i-1,j,k)
        + matrices->c.at(1,i,j,k) * matrices->p.at(0,i,j-1,k)
        + matrices->c.at(2,i,j,k) * matrices->p.at(0,i,j,k-1)
        + matrices->wrk1.at(0,i,j,k);

    double ss = (s0*matrices->a.at(3,i,j,k) - matrices->p.at(0,i,j,k)) * matrices->bnd.at(0,i,j,k);
    matrices->wrk2.at(0,i,j,k) = matrices->p.at(0,i,j,k) + OMEGA*ss;

    if (gosa != nullptr) (*gosa) += ss*ss;

}

void calculate_part( double *gosa, matrix_set_t *matrices, uint d_begin, uint d_end ) {

    for (uint r = 1; r < matrices->p.m_uiRows-1; r++) {
        for (uint c = 1; c < matrices->p.m_uiCols-1; c++) {
            for (uint d = d_begin; d < d_end; d++) calculate_at(gosa, matrices, r, c, d);
        }
    }
    
}

double jacobi( uint nn, matrix_set_t *matrices ) {

    // for the final (combined) result
    double gosa = 0.0f;

    // create thread array
    auto thread_arr = new thread[NUM_CORES];

    // the working ranges for the threads
    auto d_ranges = new uint[NUM_CORES+1];
    for (uint i=0; i<NUM_CORES; i++) d_ranges[i] = 1+i*((matrices->p.m_uiDeps-2)/NUM_CORES);
    d_ranges[NUM_CORES] = matrices->p.m_uiDeps-1;

    // the partial results
    auto gosa_arr = new double[NUM_CORES];
    fill_n(gosa_arr, NUM_CORES, 0.0f);

    // start to calculate
    for (uint n=0; n<nn; n++) {

        #if MEASURE_TIME
            ts_temp = get_timestamp();
        #endif

        // calculate in parallel
        for (uint i=0; i<NUM_CORES; i++) thread_arr[i] = thread(calculate_part, n == nn-1 ? gosa_arr+i : nullptr, matrices, d_ranges[i], d_ranges[i+1]);
        for (uint i=0; i<NUM_CORES; i++) thread_arr[i].join();

        #if MEASURE_TIME
            time_calculation += get_timestamp(ts_temp);
            ts_temp = get_timestamp();
        #endif

        // copy matrix in parallel
        for (uint i=0; i<NUM_CORES; i++) thread_arr[i] = thread(mat_float64_t::copy_partial, &matrices->wrk2, &matrices->p, 0, 1, 1, d_ranges[i], 1, matrices->p.m_uiRows-1, matrices->p.m_uiCols-1, d_ranges[i+1]);
        for (uint i=0; i<NUM_CORES; i++) thread_arr[i].join();

        #if MEASURE_TIME
            time_copying += get_timestamp(ts_temp);
        #endif
        
    }

    // sum up partial gosa
    for (uint i=0; i<NUM_CORES; i++) gosa += gosa_arr[i];

    // free ressources
    delete[] thread_arr;
    delete[] gosa_arr;
    delete[] d_ranges;

    // done
    return gosa;
}

