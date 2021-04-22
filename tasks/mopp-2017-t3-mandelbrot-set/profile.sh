#!/bin/bash
set -e;
make clean
make profile
make run
gprof mandelbrot gmon.out > profiling.txt
rm gmon.out