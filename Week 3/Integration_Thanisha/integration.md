## Week 3 – Process Scheduling Integration

Overview

The integration program brings all six process scheduling algorithms into one menu-driven application. It provides a single entry point for selecting an algorithm and viewing its results, instead of running the algorithms as separate programs.

 Integration Details

The `integration.c` file connects the scheduling implementations and displays the main menu. The user selects an option to run an algorithm or compare their performance. The integration keeps the available algorithms together in one program without requiring their individual implementations to be rewritten.

The menu includes:

1. FCFS
2. SJF Non-Preemptive
3. SJF Preemptive
4. Priority Non-Preemptive
5. Priority Preemptive
6. Round Robin
7. Performance Comparison
0. Exit

 Algorithms Included

- **FCFS:** Runs processes in arrival order.
- **SJF Non-Preemptive:** Runs the available process with the shortest burst time until it finishes.
- **SJF Preemptive:** Runs the process with the shortest remaining time and can switch when a shorter job arrives.
- **Priority Non-Preemptive:** Runs the available process with the highest priority until it finishes.
- **Priority Preemptive:** Can interrupt a running process when a higher-priority process arrives.
- **Round Robin:** Gives each process a turn using a configurable time quantum.

 Results

For an individual algorithm, the program displays a Gantt chart and a table with each process's completion time (CT), turnaround time (TAT), waiting time (WT), and response time (RT). It also displays the average waiting, turnaround, and response times, along with elapsed time, busy time, idle time, and CPU utilization.

The **Performance Comparison** option compares the results of all six algorithms using the same process details. This makes it easier to compare their waiting, turnaround, and response times, as well as CPU utilization.

Build and Run

From the Week 3 project folder, run:

```bash
./build_week3.sh
./scheduler
```

The build script compiles the project and creates the `scheduler` executable. Running it opens the process scheduling menu.
