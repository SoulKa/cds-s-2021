#!/bin/bash
set -e

time cat judge.in | MAX_CPUS=1 ./himeno >test.out 2>&1
cat test.out

time cat judge.in | ./himeno >test.out 2>&1
cat test.out