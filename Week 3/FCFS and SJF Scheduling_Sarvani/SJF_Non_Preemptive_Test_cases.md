# SJF Non-Preemptive Scheduling — Test Cases

**Algorithm:** Shortest Job First (Non-Preemptive)
Description: SJF Non-Preemptive selects the available process with the shortest burst time.

Working: The process with the shortest burst time executes first among the available processes.

Preemption: No. Once execution starts, the process continues until completion.

Advantages: Can reduce average waiting time when burst times are known.

Disadvantages: Long processes may wait for a long time, and burst time must be estimated or known.

Applications: Batch systems where process execution times can be estimated.
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
P1     0     5     9     9      4     4
P2     0     3     4     4      1     1
P3     0     1     1     1      0     0

Average Waiting Time: 1.67
Average Turnaround Time: 4.67
Average Response Time: 1.67
```

**Result:** The process with the shortest burst time executes first.

---

## Test Case 2: Processes Arrive at Different Times

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
0 -- P1 -- 7 -- P3 -- 8 -- P2 -- 12

PID    AT    BT    CT    TAT    WT    RT
P1     0     7     7     7      0     0
P2     2     4     12    10     6     6
P3     4     1     8     4      3     3

Average Waiting Time: 3.00
Average Turnaround Time: 7.00
Average Response Time: 3.00
```

**Result:** Once a process starts, it continues until completion. The shortest available process is selected next.
###Performance Metrics

Completion Time (CT): Time when a process finishes execution.

Turnaround Time (TAT): Total time from arrival to completion.

Waiting Time (WT): Total time a process spends waiting in the ready queue.

Response Time (RT): Time between arrival and the process's first CPU execution.

###Formulas

TAT = CT − AT

WT = TAT − BT

RT = First CPU Start Time − AT