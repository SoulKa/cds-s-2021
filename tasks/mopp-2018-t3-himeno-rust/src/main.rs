extern crate num_cpus;

#[allow(dead_code)]
mod matrix;

use std::io;
use std::env;
use std::ptr;
use std::thread;
use std::marker;
use std::mem;
use std::sync::Mutex;
use std::sync::Arc;
use matrix::Matrix;

// DEFINES


// FUNCTIONS

// entry point
fn main() {

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

fn calculate_partial( ptr_p : usize, ptr_wrk : usize, r_begin : usize, r_end : usize, is_last_iteration : bool ) -> f64 {

    unsafe {

        let p = ptr_p as *mut Matrix;
        let wrk = ptr_wrk as *mut Matrix;

        let cols = (*p).cols;
        let deps = (*p).deps;
        let mut gosa = 0.0;

        for r in r_begin..r_end {
            for c in 1..cols-1 {
                for d in 1..deps-1 {

                    let value = (
                        *(*p).at(r+1,c,d) + *(*p).at(r,c+1,d) + *(*p).at(r,c,d+1)
                    + *(*p).at(r-1,c,d) + *(*p).at(r,c-1,d) + *(*p).at(r,c,d-1)
                    ) / 6.0 - *(*p).at(r, c, d);
                    
                    // check if is last iteration
                    if is_last_iteration {
                        gosa += value*value;
                    } else {
                        *(*wrk).at(r, c, d) = *(*p).at(r, c, d) + 0.8*value;
                    }

                }
            }
        }

        return gosa;

    }

}

// calculates a gosa (multithreaded)
fn jacobi( rows : usize, cols : usize, deps : usize, num_iterations : u32 ) -> f64 {

    // get num of threads to use
    let num_threads : usize;
    match env::var("MAX_CPUS") {
        Ok( s ) => num_threads = s.parse().expect(""),
        Err(_) => num_threads = num_cpus::get()
    }
    eprintln!("Working with {} threads", num_threads);

    // create matrices
    let mut p = Matrix::new(rows, cols, deps);
    p.init_matrix();
    let mut wrk = Matrix::new_from_other(&p);

    // vars
    let mut gosa = 0.0;
    let mut thread_work = vec![0; num_threads+1];
    for i in 0..num_threads {
        thread_work[i] = 1+i*(p.rows-2)/num_threads;
    }
    thread_work[num_threads] = p.rows-1;

    // pointers
    let mut ptr_p = (&mut p as *mut Matrix) as usize;
    let mut ptr_wrk = (&mut wrk as *mut Matrix) as usize;
    let mut ptr_tmp : usize;

    // iterate for the given number of iterations
    for n in 0..num_iterations {

        let mut threads = Vec::new();

        // calculate full matrix
        for i in 0..num_threads {
            let r_begin = thread_work[i];
            let r_end = thread_work[i+1];
            threads.push( thread::spawn( move || calculate_partial(ptr_p, ptr_wrk, r_begin, r_end, n == num_iterations-1) ) );
        }
        for thread in threads {
            match thread.join() {
                Ok( g ) => gosa += g,
                Err( e ) => eprintln!("Error inside worker thread!")
            }
        }

        // swap matrices
        ptr_tmp = ptr_p;
        ptr_p = ptr_wrk;
        ptr_wrk = ptr_tmp;

    }

    // return result
    return gosa;

}

