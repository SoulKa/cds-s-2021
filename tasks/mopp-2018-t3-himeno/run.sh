#!/bin/bash
set -e

cat himeno.in | MAX_CPUS=1 ./himeno >test.out 2>&1
cat test.out

cat himeno.in | ./himeno >test.out 2>&1
cat test.out