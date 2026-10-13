# Week 3 – Performance Analysis

## Test Input

Three processes were used for the performance comparison. The Round Robin time quantum was 2.

| Process ID | Arrival Time | Burst Time | Priority |
|---|---:|---:|---:|
| P1 | 0 | 8 | 3 |
| P2 | 2 | 3 | 1 |
| P3 | 3 | 1 | 2 |

## Performance Comparison

| Algorithm | Average WT | Average TAT | Average RT | CPU Usage |
|---|---:|---:|---:|---:|
| FCFS | 4.67 | 8.67 | 4.67 | 100.00% |
| SJF Non-Preemptive | 4.00 | 8.00 | 4.00 | 100.00% |
| SJF Preemptive | 1.67 | 5.67 | 0.00 | 100.00% |
| Priority Non-Preemptive | 4.67 | 8.67 | 4.67 | 100.00% |
| Priority Preemptive | 2.00 | 6.00 | 0.67 | 100.00% |
| Round Robin | 3.33 | 7.33 | 1.00 | 100.00% |

WT = Waiting Time, TAT = Turnaround Time, RT = Response Time.

## Observations

- SJF Preemptive had the lowest average waiting time (1.67) and average turnaround time (5.67) in this run.
- Priority Preemptive had an average waiting time of 2.00 and an average response time of 0.67.
- Round Robin, with a time quantum of 2, had an average waiting time of 3.33 and an average response time of 1.00.
- FCFS and Priority Non-Preemptive produced the same averages for these inputs.
- CPU usage was 100% for every algorithm. The total burst time was 12 time units, and the CPU was busy from time 0 until time 12 without idle time.

## Conclusion

For this set of processes, SJF Preemptive gave the best average waiting and turnaround times. All six algorithms used the CPU continuously, so their CPU usage was 100%.
