# Himeno Benchmark

## Performance

> **Minimum Desired Speedup:** 9x or 90,51% parallel code

> **Optimal Desired Speedup:** 13x or 93,99% parallel code

**Basetime:** 768279655 microsecs

**Measurements:**
| Version | Setting | Speedup | commit/logs |
| --- | --- |  --- |
| unmodified | 1 core on server | 1x | [e8bb6522](https://cds-lab.pages.se-gitlab.inf.tu-dresden.de/cds-s-2021/cds-website/logs/c75cb293de1ed36a6bb94494fd0b3b8f3b23c2290f5a165617ce8b8dc5681fe8/2021-04-21T12:16:51+02:00.html) |
| 1 | 12 cores on PC | 5,62x or 89,69% | - |
| 2 | 1 core on server | 2,31x (to basetime) | [7979d656](https://cds-lab.pages.se-gitlab.inf.tu-dresden.de/cds-s-2021/cds-website/logs/c75cb293de1ed36a6bb94494fd0b3b8f3b23c2290f5a165617ce8b8dc5681fe8/2021-04-23T00:16:49+02:00.html) |
| 2 | 56 cores on server | 10,54x or 92,16% | [7979d656](https://cds-lab.pages.se-gitlab.inf.tu-dresden.de/cds-s-2021/cds-website/logs/c75cb293de1ed36a6bb94494fd0b3b8f3b23c2290f5a165617ce8b8dc5681fe8/2021-04-23T00:16:49+02:00.html) |
| 3 | 56 cores on server | 12,45x or ... | [...](https://cds-lab.pages.se-gitlab.inf.tu-dresden.de/cds-s-2021/cds-website/logs/c75cb293de1ed36a6bb94494fd0b3b8f3b23c2290f5a165617ce8b8dc5681fe8/2021-04-23T12:26:48+02:00.html) |

## Improvements made

- converted everything from C to C++
- made the `Matrix` struct an own class with member functions
- copying the contents form the matrix `wrk2` to `p` after every iteration in parallel
- calculating parts of the `wrk2` matrix for each iteration in parallel without locks
- reduced the preparation time of the matrices by using `std::fill_n()` to fill the matrices with initial values
- instead of copying the matrices at the end of each iteration only their pointers need to be swapped
- made all fill functions of the matrices multithreaded
- removed the use of the `wrk1` matrix as it only contains `0.0` and is never written
- substituted the many occurances of `m->at(n,r,c,d)` by precalculated memory offsets
- remove all matrices except `wrk` and `p` since they were useless
- only calculate `gosa` in the last iteration
- removed the `n` property/layer from matrices as it is unused now
- shrinked the matrices by 2 since the outmost values of each row, column or depth never change. With this change the pointers to the values that is worked on are ever-increasing
- **Important one:** Made an atomic variable that contains the next row for a thread to work on. This is different to the algorithm before because the threads work on memory that is close to one another. This should reduce cache misses when having many threads. Also the working range for each thread was badly selected. Before, every thread got a range of **depths** to work on, instead a thread should have a range of **rows** to work on since data in rows is subsequent in memory while it is not for subsequent depths.

## Problems

As already mentioned on the lab page, optimizing with partial gosa results will end in a slightly different result due to floating point precision. The input `64 64 128 10` should result in `0.003069` but results with 12 concurrent threads (and thus with 12 partial gosa sums) in `0.003070` instead.

## Bad Ideas

- using a shared variable to determine the next coordinate to work on (together with a mutex). Due to syncing this results in even worse performance with 12 threads than with a single one (~2x longer)