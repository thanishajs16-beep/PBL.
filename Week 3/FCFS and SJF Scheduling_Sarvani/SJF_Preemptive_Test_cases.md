# SJF Preemptive Scheduling — Test Cases

**Algorithm:** Shortest Remaining Time First (SRTF)

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

**Result:** With all processes arriving together, the shortest job completes first.

---

## Test Case 2: A Shorter Process Preempts a Running Process

### Input

Number of processes: `3`

| Process ID | Arrival Time | Burst Time |
|---|---:|---:|
| P1 | 0 | 8 |
| P2 | 1 | 4 |
| P3 | 2 | 2 |

### Sample Output

```text
Gantt chart:
0 -- P1 -- 1 -- P2 -- 2 -- P3 -- 4 -- P2 -- 7 -- P1 -- 14

PID    AT    BT    CT    TAT    WT    RT
P1     0     8     14    14     6     0
P2     1     4     7     6      2     0
P3     2     2     4     2      0     0

Average Waiting Time: 2.67
Average Turnaround Time: 7.33
Average Response Time: 0.00
```

**Result:** A newly arrived process with a shorter remaining time preempts the currently running process.