
#include <stdio.h>
#include "scheduling.h"

/* Validate process details and ensure every PID is unique. */
static int valid_input(Process processes[], int n,
                       GanttSegment gantt[], int capacity)
{
    if (processes == NULL || gantt == NULL ||
        n <= 0 || n > MAX_PROCESSES || capacity <= 0)
    {
        printf("Error: Invalid input or Gantt chart capacity.\n");
        return 0;
    }

    for (int i = 0; i < n; i++)
    {
        if (processes[i].pid < 0 ||
            processes[i].arrival_time < 0 ||
            processes[i].burst_time <= 0)
        {
            printf("Error: Invalid process details.\n");
            return 0;
        }

        /* Check the current PID against all earlier PIDs. */
        for (int j = 0; j < i; j++)
        {
            if (processes[i].pid == processes[j].pid)
            {
                printf("Error: Process ID must be unique. "
                       "Duplicate PID: %d\n", processes[i].pid);
                return 0;
            }
        }
    }

    return 1;
}

/* Reset calculated values before running an algorithm. */
static void reset_results(Process processes[], int n)
{
    for (int i = 0; i < n; i++)
    {
        processes[i].completion_time = 0;
        processes[i].turnaround_time = 0;
        processes[i].waiting_time = 0;
        processes[i].response_time = 0;
        processes[i].remaining_time = processes[i].burst_time;
        processes[i].first_start_time = -1;
    }
}

/* Append a Gantt segment and merge adjacent segments for the same PID. */
static int append_segment(GanttSegment gantt[], int *count,
                          int capacity, int pid, int start, int end)
{
    if (end <= start)
        return 1;

    if (*count > 0 &&
        gantt[*count - 1].pid == pid &&
        gantt[*count - 1].end_time == start)
    {
        gantt[*count - 1].end_time = end;
        return 1;
    }

    if (*count >= capacity)
        return 0;

    gantt[*count].pid = pid;
    gantt[*count].start_time = start;
    gantt[*count].end_time = end;
    (*count)++;

    return 1;
}

/*
 * Smaller priority number means higher priority.
 * Ties are resolved by earlier arrival time, then input order.
 */
static int has_higher_selection_order(const Process processes[],
                                      int a, int b)
{
    if (b == -1)
        return 1;

    if (processes[a].priority != processes[b].priority)
        return processes[a].priority < processes[b].priority;

    if (processes[a].arrival_time != processes[b].arrival_time)
        return processes[a].arrival_time < processes[b].arrival_time;

    return a < b;
}

/* Calculate completion, turnaround, waiting and response times. */
static void calculate_metrics(Process *p, int finish_time)
{
    p->completion_time = finish_time;
    p->turnaround_time = p->completion_time - p->arrival_time;
    p->waiting_time = p->turnaround_time - p->burst_time;
    p->response_time = p->first_start_time - p->arrival_time;
}

/* NON-PREEMPTIVE PRIORITY SCHEDULING */
int priority_non_preemptive(Process processes[], int n,
                            GanttSegment gantt[], int gantt_capacity)
{
    if (!valid_input(processes, n, gantt, gantt_capacity))
        return -1;

    reset_results(processes, n);

    int completed = 0;
    int time = 0;
    int gantt_count = 0;

    while (completed < n)
    {
        int selected = -1;

        /* Select the highest-priority arrived process. */
        for (int i = 0; i < n; i++)
        {
            if (processes[i].remaining_time > 0 &&
                processes[i].arrival_time <= time &&
                has_higher_selection_order(processes, i, selected))
            {
                selected = i;
            }
        }

        if (selected == -1)
        {
            /* CPU is idle until the next process arrives. */
            int next_arrival = -1;

            for (int i = 0; i < n; i++)
            {
                if (processes[i].remaining_time > 0 &&
                    (next_arrival == -1 ||
                     processes[i].arrival_time < next_arrival))
                {
                    next_arrival = processes[i].arrival_time;
                }
            }

            if (next_arrival == -1)
                break;

            if (next_arrival > time)
            {
                if (!append_segment(gantt, &gantt_count,
                                    gantt_capacity, -1,
                                    time, next_arrival))
                    return -1;

                time = next_arrival;
            }

            continue;
        }

        processes[selected].first_start_time = time;

        int start = time;
        time += processes[selected].remaining_time;
        processes[selected].remaining_time = 0;

        if (!append_segment(gantt, &gantt_count, gantt_capacity,
                            processes[selected].pid, start, time))
            return -1;

        calculate_metrics(&processes[selected], time);
        completed++;
    }

    return gantt_count;
}

/* PREEMPTIVE PRIORITY SCHEDULING */
int priority_preemptive(Process processes[], int n,
                        GanttSegment gantt[], int gantt_capacity)
{
    if (!valid_input(processes, n, gantt, gantt_capacity))
        return -1;

    reset_results(processes, n);

    int completed = 0;
    int time = 0;
    int gantt_count = 0;

    while (completed < n)
    {
        int selected = -1;

        /* Select the highest-priority process currently available. */
        for (int i = 0; i < n; i++)
        {
            if (processes[i].remaining_time > 0 &&
                processes[i].arrival_time <= time &&
                has_higher_selection_order(processes, i, selected))
            {
                selected = i;
            }
        }

        if (selected == -1)
        {
            /* No process is ready, so the CPU remains idle. */
            if (!append_segment(gantt, &gantt_count,
                                gantt_capacity, -1,
                                time, time + 1))
                return -1;

            time++;
            continue;
        }

        /* Record the first time this process gets the CPU. */
        if (processes[selected].first_start_time == -1)
        {
            processes[selected].first_start_time = time;
            processes[selected].response_time =
                time - processes[selected].arrival_time;
        }

        /* Run for one time unit, then check for higher priority arrivals. */
        if (!append_segment(gantt, &gantt_count, gantt_capacity,
                            processes[selected].pid,
                            time, time + 1))
            return -1;

        processes[selected].remaining_time--;
        time++;

        if (processes[selected].remaining_time == 0)
        {
            calculate_metrics(&processes[selected], time);
            completed++;
        }
    }

    return gantt_count;
}
