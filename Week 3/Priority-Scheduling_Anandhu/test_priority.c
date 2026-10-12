
#include <stdio.h>
#include "scheduling.h"

#define MAX_PROCESSES 5
#define MAX_SEGMENTS 100

void print_results(const char *title, Process p[], int n,
                   GanttSegment gantt[], int count)
{
    double total_wt = 0;
    double total_tat = 0;
    double total_rt = 0;

    printf("\n========== %s ==========\n", title);

    printf("\nGantt Chart:\n");
    for (int i = 0; i < count; i++) {
        printf("| P%d ", gantt[i].pid);
    }
    printf("|\n");

    printf("\nPID\tAT\tBT\tPriority\tCT\tTAT\tWT\tRT\n");

    for (int i = 0; i < n; i++) {
        printf("P%d\t%d\t%d\t%d\t\t%d\t%d\t%d\t%d\n",
               p[i].pid,
               p[i].arrival_time,
               p[i].burst_time,
               p[i].priority,
               p[i].completion_time,
               p[i].turnaround_time,
               p[i].waiting_time,
               p[i].response_time);

        total_wt += p[i].waiting_time;
        total_tat += p[i].turnaround_time;
        total_rt += p[i].response_time;
    }

    printf("\nAverage Waiting Time    = %.2f\n", total_wt / n);
    printf("Average Turnaround Time = %.2f\n", total_tat / n);
    printf("Average Response Time   = %.2f\n", total_rt / n);
}

int main(void)
{
    Process original[MAX_PROCESSES] = {
        {.pid = 1, .arrival_time = 0, .burst_time = 4, .priority = 2},
        {.pid = 2, .arrival_time = 1, .burst_time = 3, .priority = 1},
        {.pid = 3, .arrival_time = 2, .burst_time = 2, .priority = 3},
        {.pid = 4, .arrival_time = 3, .burst_time = 1, .priority = 2},
        {.pid = 5, .arrival_time = 4, .burst_time = 3, .priority = 1}
    };

    Process p1[MAX_PROCESSES];
    Process p2[MAX_PROCESSES];

    GanttSegment g1[MAX_SEGMENTS];
    GanttSegment g2[MAX_SEGMENTS];

    int n = MAX_PROCESSES;

    // Copy original input for both tests
    for (int i = 0; i < n; i++) {
        p1[i] = original[i];
        p2[i] = original[i];
    }

    int count1 = priority_non_preemptive(
        p1, n, g1, MAX_SEGMENTS
    );

    if (count1 < 0) {
        printf("Non-preemptive scheduling failed.\n");
        return 1;
    }

    print_results(
        "NON-PREEMPTIVE PRIORITY SCHEDULING",
        p1, n, g1, count1
    );

    int count2 = priority_preemptive(
        p2, n, g2, MAX_SEGMENTS
    );

    if (count2 < 0) {
        printf("Preemptive scheduling failed.\n");
        return 1;
    }

    print_results(
        "PREEMPTIVE PRIORITY SCHEDULING",
        p2, n, g2, count2
    );

    return 0;
}
