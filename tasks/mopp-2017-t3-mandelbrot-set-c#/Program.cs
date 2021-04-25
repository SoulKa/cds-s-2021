using System;
using System.Threading;
using System.Numerics;

namespace mandelbrot
{

    public struct MandelbrotParams
    {
        public readonly uint THREAD_NUMBER;
        public readonly uint NUM_THREADS;

        public MandelbrotParams(uint thread_number, uint num_threads)
        {
            THREAD_NUMBER = thread_number;
            NUM_THREADS = num_threads;
        }
    }

    class Program
    {

        static object current_pixel_mutex = new object();
        static uint current_pixel = 0;

        static uint ROWS;
        static uint COLS;
        static uint MAX_ITERATIONS;
        static char[] IMAGE;

        static uint NUM_THREADS;

        static bool GetNextPixel( ref uint next_pixel )
        {
            lock (current_pixel_mutex) next_pixel = current_pixel++;
            if (next_pixel >= ROWS*COLS) return false;
            return true;
        }

        static uint ReadNumberFromConsole()
        {
            return Convert.ToUInt32(Console.ReadLine().Trim());
        }

        static void Main(string[] args)
        {

            // get number of CPUS
            NUM_THREADS = Environment.GetEnvironmentVariable("MAX_CPUS") == null ? 1 : Convert.ToUInt32(Environment.GetEnvironmentVariable("MAX_CPUS"));
            Console.Error.WriteLine("Working with " + NUM_THREADS + " thread(s)");

            // read task parameters
            ROWS = ReadNumberFromConsole();
            COLS = ReadNumberFromConsole();
            MAX_ITERATIONS = ReadNumberFromConsole();
            var IMAGE_SIZE = ROWS*COLS;
            IMAGE = new char[IMAGE_SIZE];
            Console.Error.WriteLine("Creating a "+ROWS+"x"+COLS+" image with a maximum of "+MAX_ITERATIONS+" iterations");
            
            // let threads work
            var threads = new Thread[NUM_THREADS];
            for (uint i = 0; i < NUM_THREADS; i++) {
                threads[i] = new Thread(Mandelbrot);
                threads[i].Start(i);
            }
            for (uint i = 0; i < NUM_THREADS; i++) threads[i].Join();

            // print result
            for (uint r = 0; r < ROWS; r++) {
                for (uint c = 0; c < COLS; c++) Console.Write(IMAGE[r*COLS+c]);
                Console.Write("\n");
            }
            Console.Error.WriteLine("Done!");

        }

        static void Mandelbrot( object thread_number_uncasted )
        {

            var thread_number = (uint) thread_number_uncasted;
            uint row, col, n;
            ComplexF32 z = new ComplexF32();

            // iterate over all pixels and calculate them
            for (uint p = thread_number*ROWS*COLS/NUM_THREADS; p < (thread_number+1)*ROWS*COLS/NUM_THREADS; p++) {

                // prepare vars
                row = p / COLS;
                col = p % COLS;
                n = 0;
                z.SetZero();

                while( ++n < MAX_ITERATIONS )
                {
                    z = z
                        .Square()
                        .AddWith(
                            (float)col * 2.0f / (float)COLS - 1.5f,
                            (float)row * 2.0f / (float)ROWS - 1.0f
                        );
                }
                IMAGE[p] = z.Abs() < 2.0f ? '#' : '.';

            }

        }

    }

    class ComplexF32
    {

        public float Real { get; private set; } = 0.0f;
        public float Imaginary { get; private set; } = 0.0f;
        
        public ComplexF32( float real, float imaginary )
        {
            Real = real;
            Imaginary = imaginary;
        }

        public ComplexF32()
        {
            new ComplexF32(0.0f, 0.0f);
        }

        public void SetZero()
        {
            Real = 0.0f;
            Imaginary = 0.0f;
        }

        public ComplexF32 AddWith( ComplexF32 other )
        {
            Real = Real+other.Real;
            Imaginary = Imaginary+other.Imaginary;
            return this;
        }

        public ComplexF32 AddWith( float real, float imaginary )
        {
            Real = Real+real;
            Imaginary = Imaginary+imaginary;
            return this;
        }

        public float Abs()
        {
            if (Imaginary == 0.0f) return Real;
            if (Real == 0.0f) return Imaginary;
            return MathF.Sqrt(Imaginary*Imaginary+Real*Real);
        }

        public ComplexF32 Square()
        {
            var r = Real;
            var i = Imaginary;
            Real = r*r-i*i;
            Imaginary = 2*r*i;
            return this;
        }

    }

}
