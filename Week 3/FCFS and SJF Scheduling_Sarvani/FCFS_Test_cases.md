# FCFS Scheduling — Test Cases

**Algorithm:** First Come First Serve (FCFS)

---

## Test Case 1: All Processes Arrive at Time 0

### Input

Number of processes: `3`

| Process ID | Arrival Time | Burst Time |
|---|---:|---:|
| P1 | 0 | 5 |
| P2 | 0 | 3 |
| P3 | 0 | 1 |

### Sample Output

```text
Gantt chart:
0 -- P1 -- 5 -- P2 -- 8 -- P3 -- 9

PID    AT    BT    CT    TAT    WT    RT
P1     0     5     5     5      0     0
P2     0     3     8     8      5     5
P3     0     1     9     9      8     8

Average Waiting Time: 4.33
Average Turnaround Time: 7.33
Average Response Time: 4.33
```

**Result:** All processes execute in their input order because they arrive at time 0.

---

## Test Case 2: Processes Arrive at Different Times

### Input

Number of processes: `3`

| Process ID | Arrival Time | Burst Time |
|---|---:|---:|
| P1 | 0 | 4 |
| P2 | 2 | 3 |
| P3 | 6 | 2 |

### Sample Output

```text
Gantt chart:
0 -- P1 -- 4 -- P2 -- 7 -- P3 -- 9

PID    AT    BT    CT    TAT    WT    RT
P1     0     4     4     4      0     0
P2     2     3     7     5      2     2
P3     6     2     9     3      1     1

Average Waiting Time: 1.00
Average Turnaround Time: 4.00
Average Response Time: 1.00
```

**Result:** Processes execute in arrival order, and each process runs until completion.