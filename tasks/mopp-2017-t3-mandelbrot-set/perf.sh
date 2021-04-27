#!/bin/bash
set -e;
make clean
make

MAX_CPUS=1 perf stat -d $PWD/mandelbrot < $PWD/judge.in >/dev/null
MAX_CPUS=6 perf stat -d $PWD/mandelbrot < $PWD/judge.in >/dev/null
MAX_CPUS=12 perf stat -d $PWD/mandelbrot < $PWD/judge.in >/dev/null

if ! cmp -s test.out judge.out ; then echo "Incorrect result!"; fi