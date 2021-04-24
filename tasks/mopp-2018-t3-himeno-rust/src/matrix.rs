pub struct Matrix {
    rows : usize,
    cols : usize,
    deps : usize,
    data : Vec<f64>
}


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
            value = (r*r) as f64 / ((self.rows-1)*(self.rows-1)) as f64;
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

    pub fn copy( src : &Matrix, dst : &Matrix ) {

    }

}