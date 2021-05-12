pub struct Matrix {
    pub nums : usize,
    pub rows : usize,
    pub cols : usize,
    pub deps : usize,
    data : Vec<f64>
}

unsafe impl Send for Matrix {}
unsafe impl Sync for Matrix {}

impl Matrix {

    pub fn new( nums : usize, rows : usize, cols : usize, deps : usize ) -> Self {
        Matrix {
            nums, rows, cols, deps, data: vec![0.0; nums*rows*cols*deps]
        }
    }

    pub fn empty() -> Self {
        Matrix {
            nums: 0, rows: 0, cols: 0, deps: 0, data: vec![0.0; 0]
        }
    }

    pub fn set_values( &mut self, n : usize, value : f64 ) {
        for r in 0..self.rows {
            for c in 0..self.cols {
                for d in 0..self.deps {
                    *self.at(n, r, c, d) = value;
                }
            }
        }
    }

    pub fn init_matrix( &mut self ) {
        for r in 0..self.rows {
            for c in 0..self.cols {
                for d in 0..self.deps {
                    *self.at(0, r, c, d) = (r*r) as f64 / ((self.rows-1)*(self.rows-1)) as f64;
                }
            }
        }
    }
    
    pub fn at( &mut self, n : usize, r : usize, c : usize, d : usize ) -> &mut f64 {
        return &mut self.data[ n * self.rows * self.cols * self.deps + r * self.cols * self.deps + c * self.deps + d ];
    }

}
