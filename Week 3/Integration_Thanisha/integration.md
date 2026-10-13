# Week 3 – Process Scheduling Integration

## Overview

The integration program brings the six CPU scheduling algorithms together in one menu-driven program. Instead of running each option separately, the user can choose an algorithm from the menu, enter the process details, and view its scheduling results.

## Algorithms Included

- **FCFS:** Processes run in the order they arrive.
- **SJF Non-Preemptive:** The available process with the shortest burst time runs until it finishes.
- **SJF Preemptive:** The process with the shortest remaining time runs, and it can be interrupted when a shorter job arrives.
- **Priority Non-Preemptive:** The available process with the highest priority runs until it finishes.
- **Priority Preemptive:** A running process can be interrupted when a higher-priority process arrives.
- **Round Robin:** Processes take turns using a set time quantum.

## How It Works

The user selects an option from the menu and enters the number of processes, process IDs, arrival times, burst times, and priorities. Round Robin also asks for a time quantum. Option 7 runs a performance comparison using the same process details across all six algorithms, making their results easier to compare. Option 0 exits the program.

## Results Displayed

For an individual scheduling algorithm, the program displays a Gantt chart showing the order in which processes run, followed by a table containing:

- **CT (Completion Time):** When a process finishes.
- **TAT (Turnaround Time):** Total time from arrival to completion.
- **WT (Waiting Time):** Total time spent waiting in the ready queue.
- **RT (Response Time):** Time from arrival until the process first gets the CPU.

It also displays the average waiting, turnaround, and response times, along with total elapsed time, CPU busy time, CPU idle time, and CPU utilization. The performance comparison option presents the average metrics and CPU usage for all six algorithms in one table.

## Build and Run

From the Week 3 project folder, run:

```bash
./build_week3.sh
./scheduler
```

The build script compiles the scheduling project and creates the `scheduler` executable. Run the executable to open the menu and test the algorithms.
