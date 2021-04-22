# Himeno Benchmark

## Performance

> **Minimum Desired Speedup:** 9x or 90,51% parallel code

> **Optimal Desired Speedup:** 13x or 93,99% parallel code

**Measurements:**
| Version | Setting | Parallel code part |
| --- | --- |  --- |
| 1 | 12 cores on PC | 89,69% |

## Improvements made

- converted everything from C to C++
- made the `Matrix` struct an own class with member functions
- copying the contents form the matrix `wrk2` to `p` after every iteration in parallel
- calculating parts of the `wrk2` matrix for each iteration in parallel without locks

## Problems

As already mentioned on the lab page, optimizing with partial gosa results will end in a slightly different result due to floating point precision. The input `64 64 128 10` should result in `0.003069` but results with 12 concurrent threads (and thus with 12 partial gosa sums) in `0.003070` instead.

## Bad Ideas

- using a shared variable to determine the next coordinate to work on (together with a mutex). Due to syncing this results in even worse performance with 12 threads than with a single one (~2x longer)