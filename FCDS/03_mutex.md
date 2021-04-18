# Mutual Exclusion

## Shared Counter

A shared counter is one possibility to use when working on mutual data. Code:

```C
public class Counter {
    private long value;
    public long getAndIncrement() {
        long temp = value;  // MAKE THESE
        value = temp + 1;   // TWO LINES ATOMIC
        return temp;
    }
}
```

The counter is an object in the shared memory that guarantees *thread safe* access on the shared counter.

## Events

An Event a<sub>0</sub> can be used in an interval A = (a<sub>0</sub>, a<sub>1</sub>). Intervals may **overlap**. If they dont overlap (are disjoint), one interval **precedes** the other.

**Event ordering is:**

- **Irreflexive:** Never true that A --> A (A never preceeds A)
- **Antisymmetric:** If A --> then not true that B --> A
- **Transitive:** If A --> B and B --> C then A --> C

## Properties

**Property I: Mutual Exclusion**
If there are two different critical sections CS<sub>1</sub> and CS<sub>2</sub>, then either CS<sub>1</sub> --> CS<sub>2</sub> or CS<sub>2</sub> --> CS<sub>1</sub>. They never overlap.

**Property II+III: Deadlock-Free + Starvation-Free:**
A thread that calls `lock()` must call `unlock()` at some point.