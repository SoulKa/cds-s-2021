# Mandelbrot Set

## Performance

> **Minimum Desired Speedup:** 23x

> **Optimal Desired Speedup:** 51x

**Measurements:**
| Setting | Duration |
| --- |  --- |
| local; unmodified; 1 core | 31ms | 

## Improvements made

- added an atomic pixel pointer that gets increased by each worker thread. The stepsize gets increased depending on how much pixels are left to reduce the lock frequency
- removed the above idea in favor of the 1st work splitting idea. But this time, the condition `abs(z) < 2.0` got removed so every pixel's calculation time is equal. By this every thread has an equal amount of work to do without any need of synchronization/locks. As a nice side effect, the `abs(z)` has only be calculated once at the end of each pixel calculation which resulted in being even faster with one thread despite having more iterations in sum
- in every iteration of the inner loop the `z = ... + complex<float>( ..., ...)` code results in a new instanciation of a complex number. This could be easily be improved by calculating the complex values manually on both - the real and imagination part

## Bad Ideas

- splitting the set of pixels to work on into an **equal** amount for the threads. Not every pixel takes the same amount of time to calculate. Speedup for 12 cores was only ~3
- substituting `2.0f / constants->MAX_COL` of the term `c * 2.0f / constants->MAX_COL - 1.5f` by a precalculated constant. This results in a slightly different result since the whole term becomes `c * (2.0f / constants->MAX_COL) - 1.5f`