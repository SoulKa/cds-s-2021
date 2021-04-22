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

#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <assert.h>

using namespace std;

// CONSTANTS
const double OMEGA = 0.8;

// GLOBAL VARS
bool done = false;
mutex pos_mutex;
vec3_uint_t cur_pos(1, 1, 1);

mutex gosa_mutex;

// FUNCTIONS

bool get_next( vec3_uint_t &new_pos, mat_float64_t *m ) {

    pos_mutex.lock();
    if (done) {
        pos_mutex.unlock();
        return false;
    }

    new_pos.x = cur_pos.x;
    new_pos.y = cur_pos.y;
    new_pos.z = cur_pos.z;

    if (++cur_pos.z == m->m_uiDeps-1) {
        cur_pos.z = 1;
        if (++cur_pos.y == m->m_uiCols-1) {
            cur_pos.y = 1;
            if (++cur_pos.x == m->m_uiRows-1) {
                cur_pos.x = 1;
                done = true;
            }
        }
    }

    pos_mutex.unlock();
    return true;

}

int main() {

    uint nn, mimax, mjmax, mkmax, msize[3];

    scanf("%u", &msize[0]);
    scanf("%u", &msize[1]);
    scanf("%u", &msize[2]);
    scanf("%u", &nn);
    
    mimax = msize[0];
    mjmax = msize[1];
    mkmax = msize[2];

    // initialize matrices
    matrix_set_t matrices = {
        mat_float64_t(4,mimax,mjmax,mkmax),
        mat_float64_t(3,mimax,mjmax,mkmax),
        mat_float64_t(3,mimax,mjmax,mkmax),
        mat_float64_t(1,mimax,mjmax,mkmax),
        mat_float64_t(1,mimax,mjmax,mkmax),
        mat_float64_t(1,mimax,mjmax,mkmax),
        mat_float64_t(1,mimax,mjmax,mkmax)
    };

    matrices.p.set_init();
    matrices.bnd.set(0,1.0);
    matrices.wrk1.set(0,0.0);
    matrices.wrk2.set(0,0.0);

    matrices.a.set(0,1.0);
    matrices.a.set(1,1.0);
    matrices.a.set(2,1.0);
    matrices.a.set(3,1.0/6.0);

    matrices.b.set(0,0.0);
    matrices.b.set(1,0.0);
    matrices.b.set(2,0.0);

    matrices.c.set(0,1.0);
    matrices.c.set(1,1.0);
    matrices.c.set(2,1.0);

    // print result
    printf("%.6f\n", jacobi(nn, &matrices));
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

    //fprintf(stderr, "Workin from %u up to %u\n", d_begin, d_end);

    for (uint r = 1; r < matrices->p.m_uiRows-1; r++) {
        for (uint c = 1; c < matrices->p.m_uiCols-1; c++) {
            for (uint d = d_begin; d < d_end; d++) calculate_at(gosa, matrices, r, c, d);
        }
    }

}

double jacobi( uint nn, matrix_set_t *matrices ) {

    // get amount of cores
    auto NUM_CORES = getenv("MAX_CPUS") == nullptr ? thread::hardware_concurrency() : atoi(getenv("MAX_CPUS"));
    assert((NUM_CORES > 0 && NUM_CORES <= 56) && "Could not get the number of cores!");
    cerr << "Working with " << NUM_CORES << " cores" << endl;

    // for the final (combined) result
    double gosa = 0.0f;

    // the worker threads
    auto thread_arr = new thread[NUM_CORES];

    // the partial results
    auto gosa_arr = new double[NUM_CORES];
    fill_n(gosa_arr, NUM_CORES, 0.0f);

    // the working ranges for the threads
    auto d_ranges = new uint[NUM_CORES+1];
    for (uint i=0; i<NUM_CORES; i++) d_ranges[i] = 1+i*((matrices->p.m_uiDeps-2)/NUM_CORES);
    d_ranges[NUM_CORES] = matrices->p.m_uiDeps-1;

    // start to calculate
    for (uint n=0; n<nn; n++) {

        // calculate in parallel
        done = false;
        for (uint i=0; i<NUM_CORES; i++) thread_arr[i] = thread(calculate_part, n == nn-1 ? gosa_arr+i : nullptr, matrices, d_ranges[i], d_ranges[i+1]);
        for (uint i=0; i<NUM_CORES; i++) thread_arr[i].join();

        // copy matrix in parallel
        for (uint i=0; i<NUM_CORES; i++) thread_arr[i] = thread(mat_float64_t::copy, &matrices->wrk2, &matrices->p, 0, 1, 1, d_ranges[i], 1, matrices->p.m_uiRows-1, matrices->p.m_uiCols-1, d_ranges[i+1]);
        for (uint i=0; i<NUM_CORES; i++) thread_arr[i].join();

        //fprintf(stderr, "Iteration %u done...\n\n", n);
        
    }

    // sum up partial gosa
    for (uint i=0; i<NUM_CORES; i++) gosa += gosa_arr[i];

    delete[] thread_arr;
    delete[] gosa_arr;

    return gosa;
}

