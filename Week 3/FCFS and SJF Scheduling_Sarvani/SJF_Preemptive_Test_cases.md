# SJF Preemptive Scheduling — Test Cases

**Algorithm:** Shortest Job First (Preemptive), also called Shortest Remaining Time First (SRTF).

---

## Test Case 1: All Processes Arrive at Time 0

### Input

```text
Number of processes: 3

Enter process ID for process 1: 1
Enter AT for process 1: 0
Enter BT for process 1: 5

Enter process ID for process 2: 2
Enter AT for process 2: 0
Enter BT for process 2: 3

Enter process ID for process 3: 3
Enter AT for process 3: 0
Enter BT for process 3: 1
```

### Expected Output

```text
Gantt Chart:
0 -- P3 -- 1 -- P2 -- 4 -- P1 -- 9

PID    AT    BT    CT    TAT    WT    RT
P1     0     5     9     9      4     4
P2     0     3     4     4      1     1
P3     0     1     1     1      0     0

Average Waiting Time: 1.67
Average Turnaround Time: 4.67
Average Response Time: 1.67
```

**Result:** Since all processes arrive at time 0, the process with the shortest burst time executes first. No process is interrupted in this test case.

---

## Test Case 2: A Newly Arriving Process Preempts the Running Process

### Input

```text
Number of processes: 2

Enter process ID for process 1: 1
Enter AT for process 1: 0
Enter BT for process 1: 8

Enter process ID for process 2: 2
Enter AT for process 2: 2
Enter BT for process 2: 2
```

### Expected Output

```text
Gantt Chart:
0 -- P1 -- 2 -- P2 -- 4 -- P1 -- 10

PID    AT    BT    CT    TAT    WT    RT
P1     0     8     10    10     2     0
P2     2     2     4     2      0     0

Average Waiting Time: 1.00
Average Turnaround Time: 6.00
Average Response Time: 0.00
```

### Explanation

- At time 0, P1 starts execution with a burst time of 8.
- At time 2, P1 has 6 units remaining, and P2 arrives with a burst time of 2.
- Because P2 has the shorter remaining execution time, it preempts P1.
- P2 executes from time 2 to 4 and completes.
- P1 resumes from time 4 and completes at time 10.

**Result:** This test demonstrates actual preemption by a newly arriving process with a shorter remaining time. It also verifies that consecutive execution intervals are combined in the Gantt chart.

---

## Formulas

- Turnaround Time (TAT) = Completion Time (CT) − Arrival Time (AT)
- Waiting Time (WT) = Turnaround Time (TAT) − Burst Time (BT)
- Response Time (RT) = First CPU Start Time − Arrival Time (AT)

## Conclusion

The test cases verify shortest-remaining-time scheduling, completion times, waiting times, turnaround times, and response times. Test Case 2 specifically satisfies the recommendation to demonstrate actual preemption.