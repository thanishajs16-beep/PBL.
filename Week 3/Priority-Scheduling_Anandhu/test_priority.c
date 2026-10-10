#include <stdio.h>
#include "scheduling.h"

static void print_results(const char *title, Process p[], int n,
                          GanttSegment gantt[], int count) {
    double total_wt = 0.0;
    double total_tat = 0.0;
    double total_rt = 0.0;

    printf("\n%s\n", title);
    printf("Gantt chart:\n");
    for (int i = 0; i < count; i++) {
        if (gantt[i].pid == -1) {
            printf("| IDLE %d-%d ", gantt[i].start_time, gantt[i].end_time);
        } else {
            printf("| P%d %d-%d ", gantt[i].pid,
                   gantt[i].start_time, gantt[i].end_time);
        }
    }
    printf("|\n\n");

    printf("PID\tAT\tBT\tPriority\tCT\tTAT\tWT\tRT\n");
    for (int i = 0; i < n; i++) {
        printf("P%d\t%d\t%d\t%d\t\t%d\t%d\t%d\t%d\n",
               p[i].pid, p[i].arrival_time, p[i].burst_time, p[i].priority,
               p[i].completion_time, p[i].turnaround_time,
               p[i].waiting_time, p[i].response_time);
        total_wt += p[i].waiting_time;
        total_tat += p[i].turnaround_time;
        total_rt += p[i].response_time;
    }

    printf("Average WT  = %.2f\n", total_wt / n);
    printf("Average TAT = %.2f\n", total_tat / n);
    printf("Average RT  = %.2f\n", total_rt / n);
}

static void run_case(const char *case_name, Process input[], int n) {
    Process copy[MAX_PROCESSES];
    GanttSegment gantt[MAX_GANTT_SEGMENTS];

    for (int i = 0; i < n; i++) {
        copy[i] = input[i];
    }
    int count = priority_non_preemptive(copy, n, gantt, MAX_GANTT_SEGMENTS);
    if (count < 0) {
        printf("Non-preemptive scheduling failed for %s.\n", case_name);
        return;
    }
    print_results(case_name, copy, n, gantt, count);

    for (int i = 0; i < n; i++) {
        copy[i] = input[i];
    }
    count = priority_preemptive(copy, n, gantt, MAX_GANTT_SEGMENTS);
    if (count < 0) {
        printf("Preemptive scheduling failed for %s.\n", case_name);
        return;
    }
    print_results("Preemptive Priority", copy, n, gantt, count);
}

int main(void) {
    /* Smaller priority number = higher priority. */
    Process test1[] = {
        {.pid = 1, .arrival_time = 0, .burst_time = 4, .priority = 3},
        {.pid = 2, .arrival_time = 1, .burst_time = 3, .priority = 1},
        {.pid = 3, .arrival_time = 2, .burst_time = 2, .priority = 2}
    };

    Process test2[] = {
        {.pid = 1, .arrival_time = 0, .burst_time = 7, .priority = 3},
        {.pid = 2, .arrival_time = 2, .burst_time = 4, .priority = 1},
        {.pid = 3, .arrival_time = 3, .burst_time = 1, .priority = 2},
        {.pid = 4, .arrival_time = 5, .burst_time = 2, .priority = 1}
    };

    run_case("Test Case 1 - Non-preemptive Priority", test1, 3);
    run_case("Test Case 2 - Non-preemptive Priority", test2, 4);
    return 0;
}
