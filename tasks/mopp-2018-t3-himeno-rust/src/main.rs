extern crate num_cpus;

#[allow(dead_code)]
mod matrix;

use std::io;
use std::env;
use std::thread;
use std::sync::Arc;
use std::sync::atomic::AtomicUsize;
use std::sync::atomic::Ordering;
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

fn calculate_partial( ptr_p : usize, ptr_wrk : usize, atomic_row : Arc<AtomicUsize>, is_last_iteration : bool ) -> f64 {

    unsafe {

        let p = &mut (*(ptr_p as *mut Matrix));
        let wrk = &mut (*(ptr_wrk as *mut Matrix));

        let cols = p.cols;
        let deps = p.deps;
        let rows = p.rows;
        let mut gosa = 0.0;

        // while there is work to do (iterate over the rows via atomic)
        let mut r;
        while (r = atomic_row.fetch_add(1, Ordering::SeqCst)) == () && r < rows {
            for c in 0..cols {
                for d in 0..deps {

                    let r_i = r as isize;
                    let c_i = c as isize;
                    let d_i = d as isize;

                    let value = (
                        p.get(r_i+1,c_i,d_i) + p.get(r_i,c_i+1,d_i) + p.get(r_i,c_i,d_i+1)
                        + p.get(r_i-1,c_i,d_i) + p.get(r_i,c_i-1,d_i) + p.get(r_i,c_i,d_i-1)
                    ) / 6.0 - *p.at(r, c, d);
                    
                    // check if is last iteration
                    if is_last_iteration {
                        gosa += value*value;
                    } else {
                        *wrk.at(r, c, d) = *p.at(r, c, d) + 0.8*value;
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
    let mut p = Matrix::new(rows-2, cols-2, deps-2);
    p.init_matrix();
    let mut wrk = Matrix::new_from_other(&p);

    // vars
    let mut gosa = 0.0;

    // pointers
    let mut ptr_p = (&mut p as *mut Matrix) as usize;
    let mut ptr_wrk = (&mut wrk as *mut Matrix) as usize;
    let mut ptr_tmp : usize;

    // iterate for the given number of iterations
    for n in 0..num_iterations {

        let mut threads = Vec::new();
        let atomic_row = Arc::new( AtomicUsize::new(0) );

        // calculate full matrix
        for _ in 0..num_threads {
            let atomic_row = Arc::clone(&atomic_row);
            threads.push( thread::spawn( move || calculate_partial(ptr_p, ptr_wrk, atomic_row, n == num_iterations-1) ) );
        }
        for thread in threads {
            match thread.join() {
                Ok( g ) => gosa += g,
                Err( _e ) => eprintln!("Error inside worker thread!")
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

