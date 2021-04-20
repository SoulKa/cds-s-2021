#include <complex>
#include <iostream>
#include <thread>

#include <assert.h>

using namespace std;

int main(){

	// get amount of cores
	const auto num_cores = thread::hardware_concurrency();
	assert((num_cores > 0 && num_cores <= 56) && "Could not get the number of cores!");
	cerr << "Working with " << num_cores << " cores" << endl;

	// read stdin
	int max_row, max_column, max_n;
	cin >> max_row;
	cin >> max_column;
	cin >> max_n;

	// create image
	char **mat = (char**)malloc(sizeof(char*)*max_row);
	for (int i=0; i<max_row; i++)
		mat[i]=(char*)malloc(sizeof(char)*max_column);

	// calculate mandelbrot set
	for (int r = 0; r < max_row; ++r){
		for (int c = 0; c < max_column; ++c){
			complex<float> z = 0.0f;
			int n = 0;
			while (abs(z) < 2.0f && ++n < max_n)
				z = z*z + complex<float>(
					(float)c * 2 / max_column - 1.5f,
					(float)r * 2 / max_row - 1
				);
			mat[r][c] = (n == max_n ? '#' : '.');
		}
	}

	// print result
	for(int r = 0; r < max_row; ++r){
		for(int c = 0; c < max_column; ++c)
			std::cout << mat[r][c];
		cout << '\n';
	}	
}


