using System;

namespace mandelbrot
{

    class Program
    {

        static int ReadNumberFromConsole()
        {
            return Convert.ToInt32(Console.ReadLine().Trim());
        }

        static void Main(string[] args)
        {

            // read task parameters
            var ROWS = ReadNumberFromConsole();
            var COLS = ReadNumberFromConsole();
            var MAX_ITERATIONS = ReadNumberFromConsole();
            char[][] IMAGE = new char[ROWS][];
            for (int r = 0; r < ROWS; r++) IMAGE[r] = new char[COLS];
            
            // calculate mandelbrot set image
            for(int r = 0; r < ROWS; ++r){
                for(int c = 0; c < COLS; ++c){
                    ComplexFloat z = 0;
                    int n = 0;
                    while(ComplexFloat.Abs(z) < 2.0 && ++n < MAX_ITERATIONS)
                        z = z*z + new ComplexFloat(
                            (float)c * 2 / COLS - 1.5f,
                            (float)r * 2 / ROWS - 1.0f
                        );
                    IMAGE[r][c]=(n == MAX_ITERATIONS ? '#' : '.');
                }
            }

            // print result
            for (int r = 0; r < ROWS; r++) {
                for (int c = 0; c < COLS; c++) Console.Write(IMAGE[r][c]);
                Console.Write('\n');
            }

        }

    }

    class ComplexFloat
    {

        public float Real;
        public float Imaginary;

        public ComplexFloat( float real, float imaginary )
        {
            Real = real;
            Imaginary = imaginary;
        }

        public ComplexFloat( float values ) : this(values, values) {}

        public ComplexFloat() : this(0) {}

        public static implicit operator ComplexFloat(float f) => new ComplexFloat(f);

        public static ComplexFloat operator + (in ComplexFloat c1, in ComplexFloat c2)
        {
            return new ComplexFloat(
                c1.Real + c2.Real,
                c1.Imaginary + c2.Imaginary
            );
        }

        public static ComplexFloat operator - (in ComplexFloat c1, in ComplexFloat c2)
        {
            return new ComplexFloat(
                c1.Real - c2.Real,
                c1.Imaginary - c2.Imaginary
            );
        }

        public static ComplexFloat operator * (in ComplexFloat c1, in ComplexFloat c2)
        {
            return new ComplexFloat(
                c1.Real*c2.Real - c1.Imaginary*c2.Imaginary,
                c1.Real*c2.Imaginary + c1.Imaginary*c2.Real
            );
        }

        public static float Abs( in ComplexFloat c )
        {
            return MathF.Sqrt(c.Real*c.Real + c.Imaginary*c.Imaginary);
        }

    }

}
