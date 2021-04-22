#!/bin/bash
set -e;
make clean
make profile
make run
gprof himeno gmon.out > profiling.txt
rm gmon.out