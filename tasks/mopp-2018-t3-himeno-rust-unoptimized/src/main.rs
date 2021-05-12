extern crate num_cpus;

#[allow(dead_code)]
mod matrix;

use std::io;
use matrix::Matrix;

// DEFINES
static OMEGA : f64 = 0.8;

// FUNCTIONS

// entry point
fn main() {

    // read parameters from stdin
    let rows = read_number_from_stdin() as usize;
    let cols = read_number_from_stdin() as usize;
    let deps = read_number_from_stdin() as usize;
    let num_iterations = read_number_from_stdin();
    eprintln!("Matrix size is {}x{}x{} with {} iterations", rows, cols, deps, num_iterations);

    // create matrices
    let mut p = Matrix::new(1, rows, cols, deps);
    let mut bnd = Matrix::new(1, rows, cols, deps);
    let mut wrk1 = Matrix::new(1, rows, cols, deps);
    let mut wrk2 = Matrix::new(1, rows, cols, deps);
    let mut a = Matrix::new(4, rows, cols, deps);
    let mut b = Matrix::new(3, rows, cols, deps);
    let mut c = Matrix::new(3, rows, cols, deps);

    // initialize matrices
    p.init_matrix();
    bnd.set_values(0, 1.0);
    wrk1.set_values(0, 0.0);
    wrk2.set_values(0, 0.0);
    a.set_values(0, 1.0);
    a.set_values(1, 1.0);
    a.set_values(2, 1.0);
    a.set_values(3, 1.0/6.0);
    b.set_values(0, 0.0);
    b.set_values(1, 0.0);
    b.set_values(2, 0.0);
    c.set_values(0, 1.0);
    c.set_values(1, 1.0);
    c.set_values(2, 1.0);

    println!("{:.6}", jacobi(num_iterations, &mut a, &mut b, &mut c, &mut p, &mut bnd, &mut wrk1, &mut wrk2));

}

// helper function to read a usize from stdin
fn read_number_from_stdin() -> u32 {

    let mut l = String::new();    

    // read from stdin
    io::stdin()
        .read_line(&mut l)
        .expect("Failed to read line");
    
    // convert to usize
    return l.trim().parse().expect("Must pass valid u32 values from stdin!");

}

fn jacobi( nn : u32, a : &mut Matrix, b : &mut Matrix, c : &mut Matrix, p : &mut Matrix, bnd : &mut Matrix, wrk1 : &mut Matrix, wrk2 : &mut Matrix ) -> f64 {

    let imax : usize;
    let jmax : usize;
    let kmax : usize;
    let mut gosa = 0.0;
    let mut s0 : f64;
    let mut ss : f64;

    imax = p.rows-1;
    jmax = p.cols-1;
    kmax = p.deps-1;

    for _n in 0..nn {
        gosa = 0.0;

        for i in 1..imax {
            for j in 1..jmax {
                for k in 1..kmax {
                    s0 = (*a.at(0,i,j,k))*(*p.at(0,i+1,j,  k))
                        + (*a.at(1,i,j,k))*(*p.at(0,i,  j+1,k))
                        + (*a.at(2,i,j,k))*(*p.at(0,i,  j,  k+1))
                        + (*b.at(0,i,j,k))
                        *( (*p.at(0,i+1,j+1,k)) - (*p.at(0,i+1,j-1,k))
                        - (*p.at(0,i-1,j+1,k)) + (*p.at(0,i-1,j-1,k)) )
                        + (*b.at(1,i,j,k))
                        *( (*p.at(0,i,j+1,k+1)) - (*p.at(0,i,j-1,k+1))
                        - (*p.at(0,i,j+1,k-1)) + (*p.at(0,i,j-1,k-1)) )
                        + (*b.at(2,i,j,k))
                        *( (*p.at(0,i+1,j,k+1)) - (*p.at(0,i-1,j,k+1))
                        - (*p.at(0,i+1,j,k-1)) + (*p.at(0,i-1,j,k-1)) )
                        + (*c.at(0,i,j,k)) * (*p.at(0,i-1,j,  k))
                        + (*c.at(1,i,j,k)) * (*p.at(0,i,  j-1,k))
                        + (*c.at(2,i,j,k)) * (*p.at(0,i,  j,  k-1))
                        + (*wrk1.at(0,i,j,k));
    
                    ss = (s0*(*a.at(3,i,j,k)) - (*p.at(0,i,j,k)))*(*bnd.at(0,i,j,k));
    
                    gosa += ss*ss;
                    (*wrk2.at(0,i,j,k)) = (*p.at(0,i,j,k)) + OMEGA*ss;
                }
            }
        }

        for i in 1..imax {
            for j in 1..jmax {
                for k in 1..kmax {
                    (*p.at(0,i,j,k)) = *wrk2.at(0,i,j,k);
                }
            }
        }
        
    } /* end n loop */

    return gosa;

}

