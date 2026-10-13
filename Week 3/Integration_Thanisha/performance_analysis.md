##  Performance Analysis

For this test, three processes are scheduled using the Round Robin algorithm with a time quantum of 2.

Input:

- P1: Arrival Time = 0, Burst Time = 8, Priority = 3
- P2: Arrival Time = 2, Burst Time = 3, Priority = 1
- P3: Arrival Time = 3, Burst Time = 1, Priority = 2

Performance Comparison:

| Algorithm | Avg WT | Avg TAT | Avg RT | CPU Usage |
|---|---:|---:|---:|---:|
| FCFS | 4.67 | 8.67 | 4.67 | 100.00% |
| SJF Non-Preemptive | 4.00 | 8.00 | 4.00 | 100.00% |
| SJF Preemptive | 1.67 | 5.67 | 0.00 | 100.00% |
| Priority Non-Preemptive | 4.67 | 8.67 | 4.67 | 100.00% |
| Priority Preemptive | 2.00 | 6.00 | 0.67 | 100.00% |
| Round Robin | 3.33 | 7.33 | 1.00 | 100.00% |

WT = Waiting Time, TAT = Turnaround Time, RT = Response Time.

Observations:

- SJF Preemptive had the lowest average waiting time and turnaround time in this test.
- Priority Preemptive had an average waiting time of 2.00.
- Round Robin had an average waiting time of 3.33 with a time quantum of 2.
- FCFS and Priority Non-Preemptive gave the same results for these inputs.
- CPU usage was 100% for all algorithms because the CPU had no idle time during the run.

Conclusion:

For these inputs, SJF Preemptive gave the lowest average waiting time and turnaround time. All six algorithms had 100% CPU usage.
