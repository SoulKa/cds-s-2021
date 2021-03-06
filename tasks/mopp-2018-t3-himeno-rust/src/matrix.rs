pub struct Matrix {
    pub rows : usize,
    pub cols : usize,
    pub deps : usize,
    data : Vec<f64>
}

unsafe impl Send for Matrix {}
unsafe impl Sync for Matrix {}

impl Matrix {

    pub fn new( rows : usize, cols : usize, deps : usize ) -> Self {
        Matrix {
            rows, cols, deps, data: vec![0.0; rows*cols*deps]
        }
    }

    pub fn new_from_other( src : &Matrix ) -> Self {
        Matrix {
            rows: src.rows, cols: src.cols, deps: src.deps, data: src.data.clone()
        }
    }

    pub fn init_matrix( &mut self ) {
        let mut value : f64;
        for r in 0..self.rows {
            value = ((r+1)*(r+1)) as f64 / ((self.rows+1)*(self.rows+1)) as f64;
            for c in 0..self.cols {
                for d in 0..self.deps {
                    *self.at(r, c, d) = value;
                }
            }
        }
    }
    
    pub fn at( &mut self, r : usize, c : usize, d : usize ) -> &mut f64 {
        return &mut self.data[ r * self.cols * self.deps + c * self.deps + d ];
    }

    pub fn get( &mut self, r : isize, c : isize, d : isize ) -> f64 {
        if r == -1 { return 0.0; }
        if r == self.rows as isize { return 1.0; }
        if c == -1 || d == -1 || c == self.cols as isize || d == self.deps as isize { return ((r+1)*(r+1)) as f64 / ((self.rows+1)*(self.rows+1)) as f64; }
        return *self.at(r as usize, c as usize, d as usize);
    }

}
