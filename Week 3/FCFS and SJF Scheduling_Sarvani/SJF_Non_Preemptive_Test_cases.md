# SJF Non-Preemptive Scheduling — Test Cases

**Algorithm:** Shortest Job First (Non-Preemptive)

---

## Test Case 1: All Processes Arrive at Time 0

### Input

Number of processes: `3`

| Process ID | Arrival Time | Burst Time |
|---|---:|---:|
| P1 | 0 | 6 |
| P2 | 0 | 2 |
| P3 | 0 | 4 |

### Sample Output

```text
Gantt chart:
0 -- P2 -- 2 -- P3 -- 6 -- P1 -- 12

PID    AT    BT    CT    TAT    WT    RT
P1     0     6     12    12     6     6
P2     0     2     2     2      0     0
P3     0     4     6     6      2     2

Average Waiting Time: 2.67
Average Turnaround Time: 6.67
Average Response Time: 2.67
```

**Result:** The process with the shortest burst time executes first.

---

## Test Case 2: A Shorter Process Arrives Later

### Input

Number of processes: `3`

| Process ID | Arrival Time | Burst Time |
|---|---:|---:|
| P1 | 0 | 7 |
| P2 | 2 | 3 |
| P3 | 3 | 1 |

### Sample Output

```text
Gantt chart:
0 -- P1 -- 7 -- P3 -- 8 -- P2 -- 11

PID    AT    BT    CT    TAT    WT    RT
P1     0     7     7     7      0     0
P2     2     3     11    9      6     6
P3     3     1     8     5      4     4

Average Waiting Time: 3.33
Average Turnaround Time: 7.00
Average Response Time: 3.33
```

**Result:** Once P1 starts, it completes before the shorter processes that arrive later can execute.