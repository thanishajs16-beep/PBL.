## TEST CASE 1

Number of processes (1-100): 3

Enter Process ID for process 1: 1
Enter Arrival Time: 0
Enter Burst Time: 5
Enter Priority: 2

Enter Process ID for process 2: 2
Enter Arrival Time: 0
Enter Burst Time: 3
Enter Priority: 1

Enter Process ID for process 3: 3
Enter Arrival Time: 0
Enter Burst Time: 1
Enter Priority: 3
Enter Time Quantum: 2

Gantt chart:
0 -- P1 -- 2 -- P2 -- 4 -- P3 -- 5 -- P1 -- 7 -- P2 -- 8 -- P1 -- 9

PID     AT      BT      Priority        CT      WT      TAT     RT
P1      0       5       2               9       4       9       0
P2      0       3       1               8       5       8       2
P3      0       1       3               5       4       5       4

Average Waiting Time: 4.33
Average Turnaround Time: 7.33
Average Response Time: 2.00
Total elapsed time: 9
CPU busy time: 9
CPU idle time: 0
CPU Utilization: 100.00%



TEST CASE 2


Number of processes (1-100): 3

Enter Process ID for process 1: 1
Enter Arrival Time: 2
Enter Burst Time: 4
Enter Priority: 2

Enter Process ID for process 2: 2
Enter Arrival Time: 3
Enter Burst Time: 3
Enter Priority: 1

Enter Process ID for process 3: 3
Enter Arrival Time: 6
Enter Burst Time: 2
Enter Priority: 3
Enter Time Quantum: 2

Gantt chart:
0 -- IDLE -- 2 -- P1 -- 4 -- P2 -- 6 -- P1 -- 8 -- P3 -- 10 -- P2 -- 11

PID     AT      BT      Priority        CT      WT      TAT     RT
P1      2       4       2               8       2       6       0
P2      3       3       1               11      5       8       1
P3      6       2       3               10      2       4       2

Average Waiting Time: 3.00
Average Turnaround Time: 6.00
Average Response Time: 1.00
Total elapsed time: 11
CPU busy time: 9
CPU idle time: 2
CPU Utilization: 81.82%

Test Case 3

Number of processes (1-100): 4

Enter Process ID for process 1: 1
Enter Arrival Time: 0
Enter Burst Time: 2
Enter Priority: 1

Enter Process ID for process 2: 2 
Enter Arrival Time: 1
Enter Burst Time: 1
Enter Priority: 2

Enter Process ID for process 3: 3
Enter Arrival Time: 8
Enter Burst Time: 3
Enter Priority: 3

Enter Process ID for process 4: 4
Enter Arrival Time: 10
Enter Burst Time: 2
Enter Priority: 4
Enter Time Quantum: 2

Gantt chart:
0 -- P1 -- 2 -- P2 -- 3 -- IDLE -- 8 -- P3 -- 10 -- P4 -- 12 -- P3 -- 13

PID     AT      BT      Priority        CT      WT      TAT     RT
P1      0       2       1               2       0       2       0
P2      1       1       2               3       1       2       1
P3      8       3       3               13      2       5       0
P4      10      2       4               12      0       2       0

Average Waiting Time: 0.75
Average Turnaround Time: 2.75
Average Response Time: 0.25
Total elapsed time: 13
CPU busy time: 8
CPU idle time: 5
CPU Utilization: 61.54%
