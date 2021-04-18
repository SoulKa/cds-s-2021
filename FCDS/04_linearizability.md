# Linearizability

> Method calls are not *events*, they are *intervals* (they take time)

## FIFO Queue

A queue that has an `enqueue()` and `dequeue()` method that is thread safe.