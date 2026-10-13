# Week 3 – Process Scheduling

## Integration Test Cases

- These test cases are based on the runs made through `./scheduler` after building with `./build_week3.sh`. 
---

## Test Case 1: FCFS

**Input**

| PID | Arrival Time | Burst Time | Priority |
|---:|---:|---:|---:|
| 1 | 2 | 3 | 4 |
| 2 | 3 | 4 | 5 |
| 3 | 4 | 5 | 1 |

**Gantt chart**

```text
0 -- IDLE -- 2 -- P1 -- 5 -- P2 -- 9 -- P3 -- 14
```

**Output**

```text
PID     AT      BT      CT      TAT     WT      RT
1       2       3       5       3       0       0
2       3       4       9       6       2       2
3       4       5       14      10      5       5

Average Waiting Time: 2.33
Average Turnaround Time: 6.33
Average Response Time: 2.33
Total elapsed time: 14
CPU busy time: 12
CPU idle time: 2
CPU Utilization: 85.71%
```

**Observation:** P1 arrives first, followed by P2 and P3. FCFS runs them in arrival order, and the CPU is idle until time 2.

---

## Test Case 2: SJF Non-Preemptive

**Input**

| PID | Arrival Time | Burst Time | Priority |
|---:|---:|---:|---:|
| 1 | 2 | 3 | 4 |
| 2 | 3 | 4 | 5 |
| 3 | 5 | 3 | 5 |

**Gantt chart**

```text
0 -- IDLE -- 2 -- P1 -- 5 -- P3 -- 8 -- P2 -- 12
```

**Output**

```text
PID     AT      BT      CT      TAT     WT      RT
1       2       3       5       3       0       0
2       3       4       12      9       5       5
3       5       3       8       3       0       0

Average Waiting Time: 1.67
Average Turnaround Time: 5.00
Average Response Time: 1.67
Total elapsed time: 12
CPU busy time: 10
CPU idle time: 2
CPU Utilization: 83.33%
```

**Observation:** After P1 finishes, P2 and P3 are ready. P3 has the shorter burst time, so it runs before P2.

---

## Test Case 3: SJF Preemptive (SRTF)

**Input**

| PID | Arrival Time | Burst Time | Priority |
|---:|---:|---:|---:|
| 1 | 4 | 5 | 2 |
| 2 | 3 | 4 | 3 |
| 3 | 2 | 5 | 1 |

**Gantt chart**

```text
0 -- IDLE -- 2 -- P3 -- 7 -- P2 -- 11 -- P1 -- 16
```

**Output**

```text
PID     AT      BT      CT      TAT     WT      RT
1       4       5       16      12      7       7
2       3       4       11      8       4       4
3       2       5       7       5       0       0

Average Waiting Time: 3.67
Average Turnaround Time: 8.33
Average Response Time: 3.67
Total elapsed time: 16
CPU busy time: 14
CPU idle time: 2
CPU Utilization: 87.50%
```

**Observation:** P3 starts first because it arrives at time 2. P2 arrives at time 3, but its burst time ties with P3's remaining time, so the running process continues. P3 then finishes before P2 and P1.

---

## Test Case 4: Priority Non-Preemptive

**Input**

| PID | Arrival Time | Burst Time | Priority |
|---:|---:|---:|---:|
| 1 | 2 | 3 | 4 |
| 2 | 3 | 4 | 2 |
| 3 | 4 | 3 | 2 |

**Gantt chart**

```text
0 -- IDLE -- 2 -- P1 -- 5 -- P2 -- 9 -- P3 -- 12
```

**Output**

```text
PID     AT      BT      PR      CT      TAT     WT      RT
1       2       3       4       5       3       0       0
2       3       4       2       9       6       2       2
3       4       3       2       12      8       5       5

Average Waiting Time: 2.33
Average Turnaround Time: 5.67
Average Response Time: 2.33
Total elapsed time: 12
CPU busy time: 10
CPU idle time: 2
CPU Utilization: 83.33%
```

**Observation:** P1 runs to completion first. P2 and P3 have the same priority, so P2 runs first because it arrived earlier. In this program, a smaller priority number represents a higher priority.

---

## Test Case 5: Priority Preemptive

**Input**

| PID | Arrival Time | Burst Time | Priority |
|---:|---:|---:|---:|
| 1 | 2 | 3 | 4 |
| 2 | 3 | 4 | 2 |
| 3 | 4 | 3 | 1 |

**Gantt chart**

```text
0 -- IDLE -- 2 -- P1 -- 3 -- P2 -- 4 -- P3 -- 7 -- P2 -- 10 -- P1 -- 12
```

**Output**

```text
PID     AT      BT      PR      CT      TAT     WT      RT
1       2       3       4       12      10      7       0
2       3       4       2       10      7       3       0
3       4       3       1       7       3       0       0

Average Waiting Time: 3.33
Average Turnaround Time: 6.67
Average Response Time: 0.00
Total elapsed time: 12
CPU busy time: 10
CPU idle time: 2
CPU Utilization: 83.33%
```

**Observation:** This test demonstrates preemption. P2 interrupts P1 when it arrives with a higher priority, and P3 then interrupts P2 with an even higher priority. The unfinished processes resume later.

---

## Test Case 6: Round Robin

**Input**

| PID | Arrival Time | Burst Time | Priority |
|---:|---:|---:|---:|
| 3 | 2 | 1 | 2 |
| 2 | 1 | 2 | 5 |
| 1 | 5 | 3 | 2 |

**Time quantum:** `4`

**Gantt chart**

```text
0 -- IDLE -- 1 -- P2 -- 3 -- P3 -- 4 -- IDLE -- 5 -- P1 -- 8
```

**Output**

```text
PID     AT      BT      PR      CT      TAT     WT      RT
3       2       1       2       4       2       1       1
2       1       2       5       3       2       0       0
1       5       3       2       8       3       0       0

Average Waiting Time: 0.33
Average Turnaround Time: 2.33
Average Response Time: 0.33
Total elapsed time: 8
CPU busy time: 6
CPU idle time: 2
CPU Utilization: 75.00%
```

**Observation:** P2 runs first because it arrives at time 1. The CPU is idle between times 4 and 5 while waiting for P1. All burst times are shorter than the time quantum, so this particular run does not show a process being paused when its quantum expires.

---

## Conclusion

These six runs cover all six scheduling algorithms in the integrated menu. The Gantt charts and reported metrics are consistent with the input values.
