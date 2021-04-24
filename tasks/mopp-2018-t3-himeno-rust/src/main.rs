extern crate num_cpus;

#[allow(dead_code)]
mod matrix;

use std::io;
use std::thread;
use matrix::Matrix;

// DEFINES


// FUNCTIONS

// entry point
fn main() {

    eprintln!("Working with {} threads", num_cpus::get());

    // read parameters from stdin
    let rows = read_number_from_stdin() as usize;
    let cols = read_number_from_stdin() as usize;
    let deps = read_number_from_stdin() as usize;
    let num_iterations = read_number_from_stdin();
    eprintln!("Matrix size is {}x{}x{} with {} iterations", rows, cols, deps, num_iterations);

    println!("{:.6}", jacobi(rows, cols, deps, num_iterations));

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

// calculates a gosa (multithreaded)
fn jacobi( rows : usize, cols : usize, deps : usize, num_iterations : u32 ) -> f64 {

    // create matrices
    let mut p = &mut Matrix::new(rows, cols, deps);
    p.init_matrix();
    let mut wrk = &mut Matrix::new_from_other(&p);

    // vars
    let mut gosa = 0.0;
    let mut mat_ref_temp : &mut Matrix;
    let mut value : f64;

    // iterate for the given number of iterations
    for n in 0..num_iterations {

        // calculate full matrix
        for r in 1..rows-1 {
            for c in 1..cols-1 {
                for d in 1..deps-1 {
                    value = (
                        *p.at(r+1,c,d) + *p.at(r,c+1,d) + *p.at(r,c,d+1)
                      + *p.at(r-1,c,d) + *p.at(r,c-1,d) + *p.at(r,c,d-1)
                    ) / 6.0 - *p.at(r, c, d);
                    
                    // check if is last iteration
                    if n == num_iterations-1 {
                        gosa += value*value;
                    } else {
                        *wrk.at(r, c, d) = *p.at(r, c, d) + 0.8*value;
                    }
                }
            }
        }

        // swap matrices to prevent copying
        mat_ref_temp = wrk;
        wrk = p;
        p = mat_ref_temp;

    }

    // return result
    return gosa;

}