# Mandelbrot Set

## Performance

> **Minimum Desired Speedup:** 23x

> **Optimal Desired Speedup:** 51x

**Measurements:**
| Setting | Duration |
| --- |  --- |
| local; unmodified; 1 core | 31ms | 

## Improvements made

- created constants for `2.0f / MAX_COL` and `2.0f / MAX_ROW`
- 

## Bad Ideas

- splitting the set of pixels to work on into an **equal** amount for the threads. Not every pixel takes the same amount of time to calculate. Speedup for 12 cores was only ~3
- substituting `2.0f / constants->MAX_COL` of the term `c * 2.0f / constants->MAX_COL - 1.5f` by a precalculated constant. This results in a slightly different result since the whole term becomes `c * (2.0f / constants->MAX_COL) - 1.5f`