#!/bin/bash

i=0
while [ $i -lt $MAX_CPUS ]
do
    ./mandelbrot $i $MAX_CPUS >test_$i.out 2>/dev/null < judge.in &
    ((i++))
done

# wait for them to finish
rm -f test.out
wait

# merge results
i=0
while [ $i -lt $MAX_CPUS ]
do
    cat test_$i.out >> test.out
    ((i++))
done

# test result
if cmp -s test.out judge.out
then
    echo "Result correct!"
else
    echo "Incorrect result!"
fi