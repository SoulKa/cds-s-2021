#!/bin/bash
set -e

time cat judge.in | MAX_CPUS=1 ./himeno >test.out 2>/dev/null
if ! cmp -s test.out judge.out ; then echo "Incorrect result!"; fi

time cat judge.in | ./himeno >test.out 2>/dev/null
if ! cmp -s test.out judge.out ; then echo "Incorrect result!"; fi