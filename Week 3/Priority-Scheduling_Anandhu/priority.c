#include <stdio.h>
#include "scheduling.h"

static int valid_input(Process processes[], int n,
                       GanttSegment gantt[], int capacity) {
    if (processes == NULL || gantt == NULL ||
        n <= 0 || n > MAX_PROCESSES || capacity <= 0) {
        return 0;
    }

    for (int i = 0; i < n; i++) {
        if (processes[i].pid < 0 ||
            processes[i].arrival_time < 0 ||
            processes[i].burst_time <= 0) {
            return 0;
        }
    }
    return 1;
}

static void reset_results(Process processes[], int n) {
    for (int i = 0; i < n; i++) {
        processes[i].completion_time = 0;
        processes[i].turnaround_time = 0;
        processes[i].waiting_time = 0;
        processes[i].response_time = 0;
        processes[i].remaining_time = processes[i].burst_time;
        processes[i].first_start_time = -1;
    }
}

/* Append a segment, merging it with the previous segment when possible. */
static int append_segment(GanttSegment gantt[], int *count, int capacity,
                          int pid, int start, int end) {
    if (end <= start) {
        return 1;
    }

    if (*count > 0 &&
        gantt[*count - 1].pid == pid &&
        gantt[*count - 1].end_time == start) {
        gantt[*count - 1].end_time = end;
        return 1;
    }

    if (*count >= capacity) {
        return 0;
    }

    gantt[*count].pid = pid;
    gantt[*count].start_time = start;
    gantt[*count].end_time = end;
    (*count)++;
    return 1;
}

/*
 * Returns true if process a should be selected before process b.
 * Equal priorities: earlier arrival wins; then original input order.
 */
static int has_higher_selection_order(const Process processes[],
                                      int a, int b) {
    if (b == -1) {
        return 1;
    }
    if (processes[a].priority != processes[b].priority) {
        return processes[a].priority < processes[b].priority;
    }
    if (processes[a].arrival_time != processes[b].arrival_time) {
        return processes[a].arrival_time < processes[b].arrival_time;
    }
    return a < b;
}

static void calculate_metrics(Process *p, int finish_time) {
    p->completion_time = finish_time;
    p->turnaround_time = p->completion_time - p->arrival_time;
    p->waiting_time = p->turnaround_time - p->burst_time;
    p->response_time = p->first_start_time - p->arrival_time;
}

int priority_non_preemptive(Process processes[], int n,
                            GanttSegment gantt[], int gantt_capacity) {
    if (!valid_input(processes, n, gantt, gantt_capacity)) {
        return -1;
    }

    reset_results(processes, n);
    int completed = 0;
    int time = 0;
    int gantt_count = 0;

    while (completed < n) {
        int selected = -1;

        /* Find the highest-priority process that has already arrived. */
        for (int i = 0; i < n; i++) {
            if (processes[i].remaining_time > 0 &&
                processes[i].arrival_time <= time &&
                has_higher_selection_order(processes, i, selected)) {
                selected = i;
            }
        }

        if (selected == -1) {
            /* No process is ready: move to the next arrival time. */
            int next_arrival = -1;
            for (int i = 0; i < n; i++) {
                if (processes[i].remaining_time > 0 &&
                    (next_arrival == -1 ||
                     processes[i].arrival_time < next_arrival)) {
                    next_arrival = processes[i].arrival_time;
                }
            }

            if (next_arrival == -1) {
                break;
            }

            if (next_arrival > time) {
                if (!append_segment(gantt, &gantt_count, gantt_capacity,
                                    -1, time, next_arrival)) {
                    return -1;
                }
                time = next_arrival;
            }
            continue;
        }

        processes[selected].first_start_time = time;
        int start = time;
        time += processes[selected].remaining_time;
        processes[selected].remaining_time = 0;

        if (!append_segment(gantt, &gantt_count, gantt_capacity,
                            processes[selected].pid, start, time)) {
            return -1;
        }

        calculate_metrics(&processes[selected], time);
        completed++;
    }

    return gantt_count;
}

int priority_preemptive(Process processes[], int n,
                        GanttSegment gantt[], int gantt_capacity) {
    if (!valid_input(processes, n, gantt, gantt_capacity)) {
        return -1;
    }

    reset_results(processes, n);
    int completed = 0;
    int time = 0;
    int gantt_count = 0;

    while (completed < n) {
        int selected = -1;

        /* Choose the best process available at this time. */
        for (int i = 0; i < n; i++) {
            if (processes[i].remaining_time > 0 &&
                processes[i].arrival_time <= time &&
                has_higher_selection_order(processes, i, selected)) {
                selected = i;
            }
        }

        if (selected == -1) {
            if (!append_segment(gantt, &gantt_count, gantt_capacity,
                                -1, time, time + 1)) {
                return -1;
            }
            time++;
            continue;
        }

        if (processes[selected].first_start_time == -1) {
            processes[selected].first_start_time = time;
            processes[selected].response_time =
                time - processes[selected].arrival_time;
        }

        /* Run for one time unit; the next loop checks for new arrivals. */
        if (!append_segment(gantt, &gantt_count, gantt_capacity,
                            processes[selected].pid, time, time + 1)) {
            return -1;
        }

        processes[selected].remaining_time--;
        time++;

        if (processes[selected].remaining_time == 0) {
            calculate_metrics(&processes[selected], time);
            completed++;
        }
    }

    return gantt_count;
}
