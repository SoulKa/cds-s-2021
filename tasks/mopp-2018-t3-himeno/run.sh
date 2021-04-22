#!/bin/bash
set -e

cat judge.in | MAX_CPUS=1 ./himeno >test.out 2>&1
cat test.out

cat judge.in | ./himeno >test.out 2>&1
cat test.out