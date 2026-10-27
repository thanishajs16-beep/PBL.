# LRU and Optimal Page Replacement

This folder contains my part of the PBL Week 5 project: implementing the **LRU** and **Optimal** page replacement algorithms in Linux C.

## Files

- `common.h` — The shared header used by the team. It defines the input and result structures, algorithm function declarations, status values, and cleanup helpers.
- `lru.c` — Implements Least Recently Used (LRU). When memory is full, it replaces the page that has not been used for the longest time.
- `optimal.c` — Implements Optimal page replacement. When memory is full, it replaces the page whose next use is farthest in the future, or that will not be used again.
- `test_lru.c` — Runs LRU independently with a sample page-reference string.
- `test_optimal.c` — Runs Optimal independently with the same sample.

Both algorithm functions record the frame contents after each page request, mark each request as a hit or fault, and calculate the total hits, faults, and their percentages. They also validate the input and report errors when a simulation cannot run.

## Test on Linux

Keep these files in the same folder. Compile and run LRU:

```bash
gcc -std=c11 -Wall -Wextra -Werror -pedantic lru.c test_lru.c -o test_lru
./test_lru
```

Compile and run Optimal:

```bash
gcc -std=c11 -Wall -Wextra -Werror -pedantic optimal.c test_optimal.c -o test_optimal
./test_optimal
```

The test programs use the sample page sequence `7 0 1 2 0 3 0 4 2 3` with 3 frames. They use fixed sample input so the algorithms can be tested without the GUI.

## Sample results

| Algorithm | Page hits | Page faults | Hit ratio | Fault ratio |
|---|---:|---:|---:|---:|
| LRU | 2 | 8 | 20% | 80% |
| Optimal | 4 | 6 | 40% | 60% |

## Integration

`lru.c` provides `page_simulate_lru()` and `optimal.c` provides `page_simulate_optimal()`, using the shared declarations in `common.h`. The GUI and comparison code can call these functions with the user's page-reference string and frame count.

The two test files are only for independent testing. 