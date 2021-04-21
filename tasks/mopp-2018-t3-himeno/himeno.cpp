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

#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

using namespace std;

// CONSTANTS
const float OMEGA = 0.8;

// GLOBAL VARS
Matrix a,b,c,p,bnd,wrk1,wrk2;

// FUNCTIONS

int main() {

    unsigned int nn, mimax, mjmax, mkmax, msize[3];
    float gosa;

    scanf("%u", &msize[0]);
    scanf("%u", &msize[1]);
    scanf("%u", &msize[2]);
    scanf("%u", &nn);
    
    mimax = msize[0];
    mjmax = msize[1];
    mkmax = msize[2];

    // initialize matrices
    p.initialize(1,mimax,mjmax,mkmax);
    bnd.initialize(1,mimax,mjmax,mkmax);
    wrk1.initialize(1,mimax,mjmax,mkmax);
    wrk2.initialize(1,mimax,mjmax,mkmax);
    a.initialize(4,mimax,mjmax,mkmax);
    b.initialize(3,mimax,mjmax,mkmax);
    c.initialize(3,mimax,mjmax,mkmax);

    p.set_init();
    bnd.set(0,1.0);
    wrk1.set(0,0.0);
    wrk2.set(0,0.0);

    a.set(0,1.0);
    a.set(1,1.0);
    a.set(2,1.0);
    a.set(3,1.0/6.0);

    b.set(0,0.0);
    b.set(1,0.0);
    b.set(2,0.0);

    c.set(0,1.0);
    c.set(1,1.0);
    c.set(2,1.0);

    // calculate
    gosa = jacobi(nn, &a, &b, &c, &p, &bnd, &wrk1, &wrk2);

    // print result
    printf("%.6f\n", gosa);
    return 0;

}

float jacobi( unsigned int nn, Matrix* a, Matrix* b, Matrix* c, Matrix* p, Matrix* bnd, Matrix* wrk1, Matrix* wrk2 ) {
    float gosa, s0, ss;

    for (unsigned int n=0; n<nn; n++) {
        gosa = 0.0;

        for (unsigned int i=1; i<p->m_uiRows-1; i++) {
            for (unsigned int j=1; j<p->m_uiCols-1; j++) {
                for (unsigned int k=1; k<p->m_uiDeps-1; k++) {
                    s0 = a->at(0,i,j,k)*p->at(0,i+1,j,  k)
                        + a->at(1,i,j,k)*p->at(0,i,  j+1,k)
                        + a->at(2,i,j,k)*p->at(0,i,  j,  k+1)
                        + b->at(0,i,j,k)
                        * ( p->at(0,i+1,j+1,k) - p->at(0,i+1,j-1,k)
                        - p->at(0,i-1,j+1,k) + p->at(0,i-1,j-1,k) )
                        + b->at(1,i,j,k)
                        * ( p->at(0,i,j+1,k+1) - p->at(0,i,j-1,k+1)
                        - p->at(0,i,j+1,k-1) + p->at(0,i,j-1,k-1) )
                        + b->at(2,i,j,k)
                        * ( p->at(0,i+1,j,k+1) - p->at(0,i-1,j,k+1)
                        - p->at(0,i+1,j,k-1) + p->at(0,i-1,j,k-1) )
                        + c->at(0,i,j,k) * p->at(0,i-1,j,  k)
                        + c->at(1,i,j,k) * p->at(0,i,  j-1,k)
                        + c->at(2,i,j,k) * p->at(0,i,  j,  k-1)
                        + wrk1->at(0,i,j,k);

                    ss = (s0*a->at(3,i,j,k) - p->at(0,i,j,k)) * bnd->at(0,i,j,k);

                    gosa += ss*ss;
                    wrk2->at(0,i,j,k) = p->at(0,i,j,k) + OMEGA*ss;
                }
            }
        }

        for (unsigned int i=1 ; i<p->m_uiRows-1; i++) {
            for (unsigned int j=1 ; j<p->m_uiCols-1; j++) {
                for (unsigned int k=1; k<p->m_uiDeps-1; k++) p->at(0,i,j,k) = wrk2->at(0,i,j,k);
            }
        }
        
    }

    return gosa;
}

