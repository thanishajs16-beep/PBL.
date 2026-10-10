# Week 3 – Process Scheduling
## Student 3: Round Robin Scheduling
  
**Algorithm:** Round Robin (RR)

---

## Test Case 1: All Processes Arrive at Time 0

### Input

Number of processes: `3`

**Process 1**
- Process ID: 1
- Arrival Time: 0
- Burst Time: 5
- Priority: 2

**Process 2**
- Process ID: 2
- Arrival Time: 0
- Burst Time: 3
- Priority: 1

**Process 3**
- Process ID: 3
- Arrival Time: 0
- Burst Time: 1
- Priority: 3

**Time Quantum:** 2

### Sample Output

```text
Gantt chart:
0 -- P1 -- 2 -- P2 -- 4 -- P3 -- 5 -- P1 -- 7 -- P2 -- 8 -- P1 -- 9

PID     AT      BT      Priority        CT      WT      TAT     RT
P1      0       5       2               9       4       9       0
P2      0       3       1               8       5       8       2
P3      0       1       3               5       4       5       4

Average Waiting Time: 4.33
Average Turnaround Time: 7.33
Average Response Time: 2.00
Total elapsed time: 9
CPU busy time: 9
CPU idle time: 0
CPU Utilization: 100.00%
```

**Result:** The algorithm correctly schedules all three processes using a time quantum of 2.

---

## Test Case 2: Different Arrival Times and Initial CPU Idle Time

### Input

Number of processes: `3`

**Process 1**
- Process ID: 1
- Arrival Time: 2
- Burst Time: 4
- Priority: 2

**Process 2**
- Process ID: 2
- Arrival Time: 3
- Burst Time: 3
- Priority: 1

**Process 3**
- Process ID: 3
- Arrival Time: 6
- Burst Time: 2
- Priority: 3

**Time Quantum:** 2

### Sample Output

```text
Gantt chart:
0 -- IDLE -- 2 -- P1 -- 4 -- P2 -- 6 -- P1 -- 8 -- P3 -- 10 -- P2 -- 11

PID     AT      BT      Priority        CT      WT      TAT     RT
P1      2       4       2               8       2       6       0
P2      3       3       1               11      5       8       1
P3      6       2       3               10      2       4       2

Average Waiting Time: 3.00
Average Turnaround Time: 6.00
Average Response Time: 1.00
Total elapsed time: 11
CPU busy time: 9
CPU idle time: 2
CPU Utilization: 81.82%
```

**Result:** The algorithm correctly handles different arrival times and calculates CPU idle time and utilization.

---

## Test Case 3: Four Processes with an Intermediate CPU Idle Period

### Input

Number of processes: `4`

**Process 1**
- Process ID: 1
- Arrival Time: 0
- Burst Time: 2
- Priority: 1

**Process 2**
- Process ID: 2
- Arrival Time: 1
- Burst Time: 1
- Priority: 2

**Process 3**
- Process ID: 3
- Arrival Time: 8
- Burst Time: 3
- Priority: 3

**Process 4**
- Process ID: 4
- Arrival Time: 10
- Burst Time: 2
- Priority: 4

**Time Quantum:** 2

### Sample Output

```text
Gantt chart:
0 -- P1 -- 2 -- P2 -- 3 -- IDLE -- 8 -- P3 -- 10 -- P4 -- 12 -- P3 -- 13

PID     AT      BT      Priority        CT      WT      TAT     RT
P1      0       2       1               2       0       2       0
P2      1       1       2               3       1       2       1
P3      8       3       3               13      2       5       0
P4      10      2       4               12      0       2       0

Average Waiting Time: 0.75
Average Turnaround Time: 2.75
Average Response Time: 0.25
Total elapsed time: 13
CPU busy time: 8
CPU idle time: 5
CPU Utilization: 61.54%
```

**Result:** The algorithm correctly handles four processes, including an intermediate CPU idle period, and calculates the scheduling metrics.

---

## Conclusion

All three test cases verify Round Robin scheduling, including the Gantt chart, scheduling metrics, and CPU utilization.
