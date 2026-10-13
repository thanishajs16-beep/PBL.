 ## Week 3 - Test Cases

These are the test inputs and outputs I got while checking the six scheduling algorithms.

Test Case 1: FCFS

Input:
- P1: Arrival Time = 2, Burst Time = 3, Priority = 4
- P2: Arrival Time = 3, Burst Time = 4, Priority = 5
- P3: Arrival Time = 4, Burst Time = 5, Priority = 1

Gantt chart:
```text
0 -- IDLE -- 2 -- P1 -- 5 -- P2 -- 9 -- P3 -- 14
```

Results:

| PID | AT | BT | CT | TAT | WT | RT |
|---:|---:|---:|---:|---:|---:|---:|
| 1 | 2 | 3 | 5 | 3 | 0 | 0 |
| 2 | 3 | 4 | 9 | 6 | 2 | 2 |
| 3 | 4 | 5 | 14 | 10 | 5 | 5 |

- Average Waiting Time: 2.33
- Average Turnaround Time: 6.33
- Average Response Time: 2.33
- Total elapsed time: 14
- CPU busy time: 12
- CPU idle time: 2
- CPU utilization: 85.71%

Observation: The CPU stays idle until time 2. After that, the processes run in arrival order.

 Test Case 2: SJF Non-Preemptive

Input:
- P1: Arrival Time = 2, Burst Time = 3, Priority = 4
- P2: Arrival Time = 3, Burst Time = 4, Priority = 5
- P3: Arrival Time = 5, Burst Time = 3, Priority = 5

Gantt chart:
```text
0 -- IDLE -- 2 -- P1 -- 5 -- P3 -- 8 -- P2 -- 12
```

Results:

| PID | AT | BT | CT | TAT | WT | RT |
|---:|---:|---:|---:|---:|---:|---:|
| 1 | 2 | 3 | 5 | 3 | 0 | 0 |
| 2 | 3 | 4 | 12 | 9 | 5 | 5 |
| 3 | 5 | 3 | 8 | 3 | 0 | 0 |

- Average Waiting Time: 1.67
- Average Turnaround Time: 5.00
- Average Response Time: 1.67
- Total elapsed time: 12
- CPU busy time: 10
- CPU idle time: 2
- CPU utilization: 83.33%

Observation: P1 runs first. When it finishes, P3 is chosen because its burst time is shorter than P2's.

 Test Case 3: SJF Preemptive

Input:
- P1: Arrival Time = 4, Burst Time = 5, Priority = 2
- P2: Arrival Time = 3, Burst Time = 4, Priority = 3
- P3: Arrival Time = 2, Burst Time = 5, Priority = 1

Gantt chart:
```text
0 -- IDLE -- 2 -- P3 -- 7 -- P2 -- 11 -- P1 -- 16
```

Results:

| PID | AT | BT | CT | TAT | WT | RT |
|---:|---:|---:|---:|---:|---:|---:|
| 1 | 4 | 5 | 16 | 12 | 7 | 7 |
| 2 | 3 | 4 | 11 | 8 | 4 | 4 |
| 3 | 2 | 5 | 7 | 5 | 0 | 0 |

- Average Waiting Time: 3.67
- Average Turnaround Time: 8.33
- Average Response Time: 3.67
- Total elapsed time: 16
- CPU busy time: 14
- CPU idle time: 2
- CPU utilization: 87.50%

Observation: P3 runs first after it arrives, followed by P2 and P1.

 Test Case 4: Priority Non-Preemptive

Input:
- P1: Arrival Time = 2, Burst Time = 3, Priority = 4
- P2: Arrival Time = 3, Burst Time = 4, Priority = 2
- P3: Arrival Time = 4, Burst Time = 3, Priority = 2

Gantt chart:
```text
0 -- IDLE -- 2 -- P1 -- 5 -- P2 -- 9 -- P3 -- 12
```

Results:

| PID | AT | BT | PR | CT | TAT | WT | RT |
|---:|---:|---:|---:|---:|---:|---:|---:|
| 1 | 2 | 3 | 4 | 5 | 3 | 0 | 0 |
| 2 | 3 | 4 | 2 | 9 | 6 | 2 | 2 |
| 3 | 4 | 3 | 2 | 12 | 8 | 5 | 5 |

- Average Waiting Time: 2.33
- Average Turnaround Time: 5.67
- Average Response Time: 2.33
- Total elapsed time: 12
- CPU busy time: 10
- CPU idle time: 2
- CPU utilization: 83.33%

Observation: P1 finishes before the priority-based choice is made. P2 runs before P3 because they have the same priority and P2 arrived first.

 Test Case 5: Priority Preemptive

Input:
- P1: Arrival Time = 2, Burst Time = 3, Priority = 4
- P2: Arrival Time = 3, Burst Time = 4, Priority = 2
- P3: Arrival Time = 4, Burst Time = 3, Priority = 1

Gantt chart:
```text
0 -- IDLE -- 2 -- P1 -- 3 -- P2 -- 4 -- P3 -- 7 -- P2 -- 10 -- P1 -- 12
```

Results:

| PID | AT | BT | PR | CT | TAT | WT | RT |
|---:|---:|---:|---:|---:|---:|---:|---:|
| 1 | 2 | 3 | 4 | 12 | 10 | 7 | 0 |
| 2 | 3 | 4 | 2 | 10 | 7 | 3 | 0 |
| 3 | 4 | 3 | 1 | 7 | 3 | 0 | 0 |

- Average Waiting Time: 3.33
- Average Turnaround Time: 6.67
- Average Response Time: 0.00
- Total elapsed time: 12
- CPU busy time: 10
- CPU idle time: 2
- CPU utilization: 83.33%

Observation: P2 interrupts P1 when it arrives, and P3 then interrupts P2 because it has the higher priority.

 Test Case 6: Round Robin

Input:
- P3: Arrival Time = 2, Burst Time = 1, Priority = 2
- P2: Arrival Time = 1, Burst Time = 2, Priority = 5
- P1: Arrival Time = 5, Burst Time = 3, Priority = 2
- Time quantum: 4

Gantt chart:
```text
0 -- IDLE -- 1 -- P2 -- 3 -- P3 -- 4 -- IDLE -- 5 -- P1 -- 8
```

Results:

| PID | AT | BT | PR | CT | TAT | WT | RT |
|---:|---:|---:|---:|---:|---:|---:|---:|
| 3 | 2 | 1 | 2 | 4 | 2 | 1 | 1 |
| 2 | 1 | 2 | 5 | 3 | 2 | 0 | 0 |
| 1 | 5 | 3 | 2 | 8 | 3 | 0 | 0 |

- Average Waiting Time: 0.33
- Average Turnaround Time: 2.33
- Average Response Time: 0.33
- Total elapsed time: 8
- CPU busy time: 6
- CPU idle time: 2
- CPU utilization: 75.00%

Observation: Each process finishes within one time quantum in this test, so none needs another turn.

## Conclusion

I tested all six scheduling options and recorded their Gantt charts and timing results. CPU utilization is different across these examples because the CPU has different amounts of idle time.
