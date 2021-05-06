using System;
using System.Threading;

namespace mandelbrot
{

    struct MandelbrotParams
    {
        public readonly uint THREAD_NUMBER;
        public InputQueue iq;
        public OutputQueue oq;


        public MandelbrotParams( uint thread_number )
        {
            THREAD_NUMBER = thread_number;
            iq = new InputQueue();
            oq = new OutputQueue();
        }
    }

    class Program
    {

        static uint ROWS;
        static uint COLS;
        static uint MAX_ITERATIONS;
        static char[] IMAGE;
        static bool DONE = false;

        static uint NUM_THREADS;

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

            // create worker thread params
            var thread_params = new MandelbrotParams[NUM_THREADS];
            for (uint i = 0; i < NUM_THREADS; i++) thread_params[i] = new MandelbrotParams(i);

            // start input thread
            var input_thread = new Thread(ProvideInput);
            input_thread.Start(thread_params);
            
            // let threads work
            var threads = new Thread[NUM_THREADS];
            for (uint i = 0; i < NUM_THREADS; i++) {
                threads[i] = new Thread(Mandelbrot);
                threads[i].Start(thread_params[i]);
            }

            // start output thread
            var output_thread = new Thread(CollectOutput);
            output_thread.Start(thread_params);

            // join threads
            input_thread.Join();
            Console.Error.WriteLine("Finished providing the input, waiting for output thread...");
            output_thread.Join();
            Console.Error.WriteLine("Finished collecting the output, waiting for worker threads...");
            DONE = true;
            for (uint i = 0; i < NUM_THREADS; i++) threads[i].Join();

            // print result
            for (uint r = 0; r < ROWS; r++) Console.WriteLine(IMAGE, (int)(r*COLS), (int)COLS);
            Console.Error.WriteLine("Done!");

        }

        // CALCULATES THE MANDELBROT SET
        static void Mandelbrot( object params_uncasted )
        {

            var prms = (MandelbrotParams) params_uncasted;
            uint row, col, n, p;
            float z_r, z_i,
                z_r_sqr, z_i_sqr,
                c_r, c_i,
                tmp;

            while ( true ) {

                // get next pixel from input queue
                prms.iq.Pop( out p, in DONE );
                if (DONE) break;

                // prepare vars
                row = p / COLS;
                col = p % COLS;

                n = 0u;
                z_r = 0f;
                z_i = 0f;
                c_r = col * 2.0f / COLS - 1.5f;
                c_i = row * 2.0f / ROWS - 1.0f;    

                // calculate pixel
                while (( (z_r_sqr = (z_r*z_r)) + (z_i_sqr = (z_i*z_i)) ) < 4.0f && ++n < MAX_ITERATIONS) {
                    tmp = z_r;
                    z_r = z_r_sqr - z_i_sqr + c_r;
                    z_i = z_i * 2.0f * tmp  + c_i;
                }

                // add to output queue
                prms.oq.Push( in p, n == MAX_ITERATIONS ? '#' : '.' );

            }

        }

        // PROVIDES PIXEL NUMBERS TO WORK ON
        static void ProvideInput( object worker_params_array_uncasted )
        {

            // cast params
            var wp_arr = (MandelbrotParams[]) worker_params_array_uncasted;

            // keep providing new pixels until no more left
            UInt32 p = 0;
            while (p < IMAGE.Length) {

                for (var i = 0; i < NUM_THREADS; i++) {

                    while (!wp_arr[i].iq.IsFull()) {
                        wp_arr[i].iq.Push( in p );
                        p++;
                        if (p == IMAGE.Length) return;
                    }

                }

            }

        }

        // COLLECTS CALCULATED PIXELS
        static void CollectOutput( object worker_params_array_uncasted )
        {

            // cast params
            var wp_arr = (MandelbrotParams[]) worker_params_array_uncasted;

            // keep collecting output of workers until image done
            UInt32 p_count = 0, p;
            char v;
            while (p_count < IMAGE.Length) {

                for (var i = 0; i < NUM_THREADS; i++) {
                    while (!wp_arr[i].oq.IsEmpty()) {
                        wp_arr[i].oq.Pop( out p, out v );
                        IMAGE[p] = v;
                        p_count++;
                        
                    }
                }

            }

        }

    }

    // AN INPUT QUEUE. It's thread safe for specific use cases
    class InputQueue {

        private UInt16 first_index = 0;
        private UInt16 last_index = 0;
        private UInt32[] data;

        public InputQueue( UInt16 size = 16 ) {
            data = new UInt32[size];
            Array.Fill(data, UInt32.MaxValue);
        }

        public void Pop( out UInt32 out_value, in bool abort_flag ) {
            while (data[first_index] == UInt32.MaxValue && !abort_flag);
            out_value = data[first_index];
            data[first_index] = UInt32.MaxValue;
            first_index = (UInt16)((first_index + 1) % data.Length);
        }

        public void Push( in UInt32 in_value ) {
            data[last_index] = in_value;
            last_index = (UInt16)((last_index + 1) % data.Length);
        }

        public bool IsFull() {
            return data[last_index] != UInt32.MaxValue;
        }

    }

    // AN OUTPUT QUEUE. It's thread safe for specific use cases
    class OutputQueue {

        public struct PixelValue {
            public UInt32 Number;
            public char Value;
        }

        private UInt16 first_index = 0;
        private UInt16 last_index = 0;
        private PixelValue[] data;

        public OutputQueue( UInt16 size = 8 ) {
            data = new PixelValue[size];
            for (var i = 0; i < data.Length; i++) data[i].Number = UInt32.MaxValue;
        }

        public void Pop( out UInt32 out_pixel_number, out char out_pixel_value ) {
            out_pixel_number = data[first_index].Number;
            out_pixel_value = data[first_index].Value;
            data[first_index].Number = UInt32.MaxValue;
            first_index = (UInt16)((first_index + 1) % data.Length);
        }

        public void Push( in UInt32 in_pixel_number, in char in_pixel_value ) {
            while (data[last_index].Number != UInt32.MaxValue);
            data[last_index].Value = in_pixel_value;
            data[last_index].Number = in_pixel_number;
            last_index = (UInt16)((last_index + 1) % data.Length);
        }

        public bool IsEmpty() {
            return data[first_index].Number == UInt32.MaxValue;
        }

    }

}
