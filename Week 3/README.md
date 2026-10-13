## Week 3 – Process Scheduling

 Overview

This project implements a process scheduling simulator in C. It demonstrates how different scheduling algorithms select processes for execution and how those choices affect scheduling performance. A menu lets users run individual algorithms or compare all six algorithms using the same set of process details.

 Scheduling Algorithms

The simulator supports:

- **FCFS (First Come First Served):** Runs processes in arrival order.
- **SJF Non-Preemptive:** Selects the available process with the shortest burst time and lets it finish before switching.
- **SJF Preemptive (SRTF):** Selects the process with the shortest remaining execution time and can interrupt a running process.
- **Priority Non-Preemptive:** Selects a process based on priority and lets it finish before switching.
- **Priority Preemptive:** Can interrupt a running process when another process with a higher scheduling priority becomes available.
- **Round Robin (RR):** Gives each process a time slice based on a configurable time quantum.

 Input and Output

For each process, the simulator uses its process ID, arrival time, burst time, and priority. Round Robin also requires a time quantum.

The results include a Gantt chart showing the order and timing of execution, along with the following metrics:

- **Completion Time (CT):** When the process finishes.
- **Turnaround Time (TAT):** Completion time minus arrival time.
- **Waiting Time (WT):** Turnaround time minus burst time.
- **Response Time (RT):** Time from arrival until the process first gets the CPU.
- **Average metrics and CPU utilization:** Summaries used to review scheduling performance.

 Performance Comparison

The performance comparison runs all six algorithms with the same process workload and displays their average waiting time, turnaround time, response time, and CPU utilization. Using a common workload makes it easier to observe how the algorithms differ. The results depend on the processes' arrival times, burst times, priorities, and the Round Robin time quantum.

 Build and Run

Open a terminal in the project directory and run:

```bash
./build_week3.sh
./scheduler
```

Choose an option from the menu, then enter the requested process details. Select **Performance Comparison** to compare all six algorithms using one common workload. Select `0` to exit.

## Testing

The test cases cover the scheduling algorithms and their outputs, including Gantt charts and scheduling metrics. The performance analysis summarizes observations from comparing the algorithms.
