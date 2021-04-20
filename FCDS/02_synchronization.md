# Synchronization

**Two types of formal properties:**
- Safety Properties: Nothing bad happens
- Liveness Properties: Something good happens eventually

**Formal Problems of Ressource sharing:**
- Mutual Exclusion
  - Both "pets" never in pond at same time
  - This is a *safety* property
- No Deadlock
  - If one currently wants to get in pond ==> let it in
  - If both want in at same time ==> let one in
  - This is a *liveness property*

But: Threads cannot "see" what other threads are doing/ when something is being occupied. We need something explicitly for communicating the coordination.