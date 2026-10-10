#ifndef SCHEDULING_H
#define SCHEDULING_H

#define MAX_PROCESSES 100
#define MAX_GANTT_SEGMENTS 10000

typedef struct {
    int pid;
    int arrival_time;
    int burst_time;
    int priority;

    /* These fields are filled by the scheduling algorithm. */
    int completion_time;
    int turnaround_time;
    int waiting_time;
    int response_time;
    int remaining_time;
    int first_start_time;
} Process;

typedef struct {
    int pid;        /* pid == -1 means CPU is idle */
    int start_time;
    int end_time;
} GanttSegment;

/*
 * Returns the number of Gantt segments on success, or -1 if the
 * supplied Gantt array is too small or the input is invalid.
 *
 * Convention: a smaller priority number means higher priority.
 * Processes with equal priority are ordered by earlier arrival time;
 * if both are equal, original input order is preserved.
 */
int priority_non_preemptive(
    Process processes[], int n,
    GanttSegment gantt[], int gantt_capacity
);

int priority_preemptive(
    Process processes[], int n,
    GanttSegment gantt[], int gantt_capacity
);

#endif
