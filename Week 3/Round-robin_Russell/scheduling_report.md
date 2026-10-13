# Week 3 – Round Robin Scheduling Report

The Round Robin program was tested with three cases using a time quantum of 2. The results show how arrival times and the ready queue affect waiting time, turnaround time, response time, and CPU utilization.

## Observations

**Test 1 – All processes arrive at time 0**

The CPU stays busy until all processes finish, so utilization is 100%. The average waiting time is 4.33, average turnaround time is 7.33, and average response time is 2.00. Each process gets a turn for up to 2 time units, and unfinished processes return to the ready queue.

**Test 2 – Different arrival times**

The CPU is idle from time 0 to 2 because no process has arrived yet. After that, the processes keep the CPU busy until completion. The total idle time is 2 and CPU utilization is 81.82%. The average waiting time is 3.00, average turnaround time is 6.00, and average response time is 1.00.

**Test 3 – An idle period between processes**

After the first two processes finish, the CPU remains idle from time 3 to 8 while waiting for the next process. This gives 5 units of idle time and a CPU utilization of 61.54%. The average waiting time is 0.75, average turnaround time is 2.75, and average response time is 0.25. These averages are lower because the processes have short burst times and spend little time waiting in the ready queue.

## Overall observation

The results show that CPU utilization depends on whether a process is ready to run, not just on the scheduling algorithm. Waiting, turnaround, and response times depend on each process's arrival time, burst time, and position in the ready queue. The program accepts priority values, but Round Robin scheduling does not use priority to decide the order; it follows the ready queue and the time quantum.
