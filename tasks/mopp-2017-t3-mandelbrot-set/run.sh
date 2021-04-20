#!/bin/bash
set -e

time cat mandelbrot.in | MAX_CPUS=1 ./mandelbrot 2>&1
time cat mandelbrot.in | ./mandelbrot 2>&1