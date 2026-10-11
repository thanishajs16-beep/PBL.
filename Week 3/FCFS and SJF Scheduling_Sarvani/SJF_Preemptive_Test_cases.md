# SJF Preemptive Scheduling — Test Cases

**Algorithm:** Shortest Job First (Preemptive), also called Shortest Remaining Time First (SRTF)

---

## Test Case 1: All Processes Arrive at Time 0

### Input

```text
Number of processes: 3

Enter process ID for process 1: P1
Enter AT for process 1: 0
Enter BT for process 1: 5

Enter process ID for process 2: P2
Enter AT for process 2: 0
Enter BT for process 2: 3

Enter process ID for process 3: P3
Enter AT for process 3: 0
Enter BT for process 3: 1
```

### Sample Output

```text
Gantt chart:
0 -- P3 -- 1 -- P2 -- 4 -- P1 -- 9

PID    AT    BT    CT    TAT    WT    RT
P1     0     5     9     9      4     0
P2     0     3     4     4      1     0
P3     0     1     1     1      0     0

Average Waiting Time: 1.67
Average Turnaround Time: 4.67
Average Response Time: 0.00
```

**Result:** The shortest remaining burst time is selected. With all processes arriving together, the shortest job executes first.

---

## Test Case 2: A Shorter Process Arrives During Execution

### Input

```text
Number of processes: 3

Enter process ID for process 1: P1
Enter AT for process 1: 0
Enter BT for process 1: 7

Enter process ID for process 2: P2
Enter AT for process 2: 2
Enter BT for process 2: 4

Enter process ID for process 3: P3
Enter AT for process 3: 4
Enter BT for process 3: 1
```

### Sample Output

```text
Gantt chart:
0 -- P1 -- 2 -- P2 -- 4 -- P3 -- 5 -- P2 -- 7 -- P1 -- 12

PID    AT    BT    CT    TAT    WT    RT
P1     0     7     12    12     5     0
P2     2     4     7     5      1     0
P3     4     1     5     1      0     0

Average Waiting Time: 2.00
Average Turnaround Time: 6.00
Average Response Time: 0.00
```

**Result:** If a newly arrived process has a shorter remaining time, it can interrupt the running process.
