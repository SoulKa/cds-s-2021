# Himeno Benchmark

## Performance

> **Minimum Desired Speedup:** 9x

> **Optimal Desired Speedup:** 13x

**Measurements:**
| Setting | Duration |
| --- |  --- |
## Improvements made

- converted everything from C to C++
- made the `Matrix` struct an own class with member functions

## Problems

As already mentioned on the lab page, optimizing with partial gosa results will end in a slightly different result due to floating point precision. The input `64 64 128 10` should result in `0.003069` but results with 12 concurrent threads (and thus with 12 partial gosa sums) in `0.003070` instead.

## Bad Ideas

- using a shared variable to determine the next coordinate to work on (together with a mutex). Due to syncing this results in even worse performance with 12 threads than with a single one (~2x longer)