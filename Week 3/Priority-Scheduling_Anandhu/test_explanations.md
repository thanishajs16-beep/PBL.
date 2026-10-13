# Test Case Explanations — Priority Scheduling

## Test 1: Different priorities
All processes arrive at time 0 but have different priorities. This checks whether the scheduler chooses processes according to the priority rule. Compare the Gantt chart and calculated CT, TAT, WT, and RT values with the program output.

## Test 2: Different arrival times
Processes arrive at different times. This checks arrival-time handling and the difference between non-preemptive and preemptive priority scheduling. In non-preemptive scheduling, a running process continues until completion. In preemptive scheduling, a newly arrived higher-priority process can interrupt it.

## Test 3: Duplicate process ID
The ID P1 is entered twice. Process IDs must be unique. The program should display a validation error and stop scheduling the invalid input.

## Before submission
Run all three cases in both scheduling modes where applicable. Copy the real console output into `outputs.txt` and, if your instructor requires screenshots, save screenshots of the terminal output as well. Make sure the priority convention matches your implementation.
