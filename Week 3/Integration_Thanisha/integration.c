#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_PROCESSES 100

typedef struct {
    int pid, at, bt, priority;
    int remaining, first_start, ct, tat, wt, rt;
} SchedProcess;

typedef struct {
    int pid, start, end;
} GanttSegment;

typedef struct {
    GanttSegment *segments;
    int count, capacity;
    double avg_wt, avg_tat, avg_rt, cpu_util;
    int elapsed, busy, idle;
} ScheduleResult;

static int append_segment(ScheduleResult *r, int pid, int start, int end)
{
    if (start >= end)
        return 1;

    if (r->count > 0) {
        GanttSegment *last = &r->segments[r->count - 1];
        if (last->pid == pid && last->end == start) {
            last->end = end;
            return 1;
        }
    }

    if (r->count == r->capacity) {
        int cap = r->capacity ? r->capacity * 2 : 16;
        GanttSegment *s = realloc(r->segments, (size_t)cap * sizeof(*s));
        if (!s)
            return 0;
        r->segments = s;
        r->capacity = cap;
    }

    r->segments[r->count++] = (GanttSegment){pid, start, end};
    return 1;
}

static void free_result(ScheduleResult *r)
{
    free(r->segments);
    r->segments = NULL;
    r->count = r->capacity = 0;
}

static int find_next_arrival(SchedProcess p[], int n, int time)
{
    int next = -1;

    for (int i = 0; i < n; i++)
        if (p[i].remaining > 0 && p[i].at > time &&
            (next == -1 || p[i].at < next))
            next = p[i].at;

    return next;
}

static int select_process(SchedProcess p[], int n, int time, int alg)
{
    int best = -1;

    for (int i = 0; i < n; i++) {
        if (p[i].remaining <= 0 || p[i].at > time)
            continue;

        if (best == -1) {
            best = i;
            continue;
        }

        int choose = 0;

        if (alg == 1)
            choose = p[i].at < p[best].at;
        else if (alg == 2)
            choose = p[i].bt < p[best].bt ||
                    (p[i].bt == p[best].bt && p[i].at < p[best].at);
        else if (alg == 3)
            choose = p[i].remaining < p[best].remaining ||
                    (p[i].remaining == p[best].remaining &&
                     p[i].at < p[best].at);
        else if (alg == 4 || alg == 5) {
            choose = p[i].priority < p[best].priority ||
                    (p[i].priority == p[best].priority &&
                     p[i].at < p[best].at);

            if (alg == 5 && p[i].priority == p[best].priority &&
                p[i].at == p[best].at &&
                p[i].remaining < p[best].remaining)
                choose = 1;
        }

        if (choose)
            best = i;
    }

    return best;
}

static void calculate_metrics(SchedProcess p[], int n, ScheduleResult *r)
{
    double wt = 0, tat = 0, rt = 0;
    int finish = 0, busy = 0;

    for (int i = 0; i < n; i++) {
        p[i].tat = p[i].ct - p[i].at;
        p[i].wt = p[i].tat - p[i].bt;
        p[i].rt = p[i].first_start - p[i].at;

        wt += p[i].wt;
        tat += p[i].tat;
        rt += p[i].rt;
        busy += p[i].bt;

        if (p[i].ct > finish)
            finish = p[i].ct;
    }

    r->avg_wt = wt / n;
    r->avg_tat = tat / n;
    r->avg_rt = rt / n;
    r->elapsed = finish;
    r->busy = busy;
    r->idle = finish - busy;
    r->cpu_util = finish > 0 ? 100.0 * busy / finish : 0.0;
}

static int run_algorithm(SchedProcess input[], int n, int alg,
                         int quantum, ScheduleResult *r, int show)
{
    SchedProcess p[MAX_PROCESSES];
    int time = 0, completed = 0;

    memset(r, 0, sizeof(*r));
    memcpy(p, input, (size_t)n * sizeof(*p));

    for (int i = 0; i < n; i++) {
        p[i].remaining = p[i].bt;
        p[i].first_start = -1;
        p[i].ct = 0;
    }

    if (alg == 6) {
        int queue[MAX_PROCESSES + 1];
        int front = 0, rear = 0, count = 0;
        int arrived[MAX_PROCESSES] = {0};

        while (completed < n) {
            for (int i = 0; i < n; i++) {
                if (!arrived[i] && p[i].at <= time) {
                    queue[rear] = i;
                    rear = (rear + 1) % (MAX_PROCESSES + 1);
                    count++;
                    arrived[i] = 1;
                }
            }

            if (count == 0) {
                int next = find_next_arrival(p, n, time);
                if (next == -1)
                    break;
                if (!append_segment(r, -1, time, next)) {
                    free_result(r);
                    return 0;
                }
                time = next;
                continue;
            }

            int i = queue[front];
            front = (front + 1) % (MAX_PROCESSES + 1);
            count--;

            if (p[i].first_start == -1)
                p[i].first_start = time;

            int run = p[i].remaining < quantum ?
                      p[i].remaining : quantum;

            if (!append_segment(r, p[i].pid, time, time + run)) {
                free_result(r);
                return 0;
            }

            time += run;
            p[i].remaining -= run;

            for (int j = 0; j < n; j++) {
                if (!arrived[j] && p[j].at <= time) {
                    queue[rear] = j;
                    rear = (rear + 1) % (MAX_PROCESSES + 1);
                    count++;
                    arrived[j] = 1;
                }
            }

            if (p[i].remaining == 0) {
                p[i].ct = time;
                completed++;
            } else {
                queue[rear] = i;
                rear = (rear + 1) % (MAX_PROCESSES + 1);
                count++;
            }
        }
    } else {
        while (completed < n) {
            int i = select_process(p, n, time, alg);

            if (i == -1) {
                int next = find_next_arrival(p, n, time);
                if (next == -1)
                    break;
                if (!append_segment(r, -1, time, next)) {
                    free_result(r);
                    return 0;
                }
                time = next;
                continue;
            }

            SchedProcess *cur = &p[i];

            if (cur->first_start == -1)
                cur->first_start = time;

            int run = (alg == 1 || alg == 2 || alg == 4) ?
                      cur->remaining : 1;

            if (!append_segment(r, cur->pid, time, time + run)) {
                free_result(r);
                return 0;
            }

            time += run;
            cur->remaining -= run;

            if (cur->remaining == 0) {
                cur->ct = time;
                completed++;
            }
        }
    }

    calculate_metrics(p, n, r);

    if (show) {
        printf("\nGantt chart:\n");

        if (r->count > 0) {
            printf("%d", r->segments[0].start);
            for (int i = 0; i < r->count; i++) {
                if (r->segments[i].pid == -1)
                    printf(" -- IDLE -- %d", r->segments[i].end);
                else
                    printf(" -- P%d -- %d",
                           r->segments[i].pid, r->segments[i].end);
            }
            printf("\n");
        }

        if (alg >= 4)
            printf("\nPID\tAT\tBT\tPR\tCT\tTAT\tWT\tRT\n");
        else
            printf("\nPID\tAT\tBT\tCT\tTAT\tWT\tRT\n");

        for (int i = 0; i < n; i++) {
            if (alg >= 4)
                printf("%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\n",
                       p[i].pid, p[i].at, p[i].bt, p[i].priority,
                       p[i].ct, p[i].tat, p[i].wt, p[i].rt);
            else
                printf("%d\t%d\t%d\t%d\t%d\t%d\t%d\n",
                       p[i].pid, p[i].at, p[i].bt,
                       p[i].ct, p[i].tat, p[i].wt, p[i].rt);
        }

        printf("\nAverage Waiting Time: %.2f\n", r->avg_wt);
        printf("Average Turnaround Time: %.2f\n", r->avg_tat);
        printf("Average Response Time: %.2f\n", r->avg_rt);
        printf("Total elapsed time: %d\n", r->elapsed);
        printf("CPU busy time: %d\n", r->busy);
        printf("CPU idle time: %d\n", r->idle);
        printf("CPU Utilization: %.2f%%\n", r->cpu_util);
    }

    return 1;
}

static int read_processes(SchedProcess p[], int *n, int need_priority)
{
    printf("Enter the number of processes: ");

    if (scanf("%d", n) != 1 || *n < 1 || *n > MAX_PROCESSES) {
        printf("Invalid number of processes.\n");
        return 0;
    }

    for (int i = 0; i < *n; i++) {
        printf("\nProcess %d\n", i + 1);
        printf("Process ID: ");

        if (scanf("%d", &p[i].pid) != 1 || p[i].pid <= 0) {
            printf("Invalid process ID.\n");
            return 0;
        }

        for (int j = 0; j < i; j++) {
            if (p[j].pid == p[i].pid) {
                printf("Process IDs must be unique.\n");
                return 0;
            }
        }

        printf("Arrival Time: ");
        if (scanf("%d", &p[i].at) != 1 || p[i].at < 0) {
            printf("Arrival time must be non-negative.\n");
            return 0;
        }

        printf("Burst Time: ");
        if (scanf("%d", &p[i].bt) != 1 || p[i].bt <= 0) {
            printf("Burst time must be greater than zero.\n");
            return 0;
        }

        p[i].priority = 0;

        if (need_priority) {
            printf("Priority: ");
            if (scanf("%d", &p[i].priority) != 1) {
                printf("Invalid priority.\n");
                return 0;
            }
        }
    }

    return 1;
}

static void run_individual(int alg)
{
    SchedProcess p[MAX_PROCESSES];
    int n, quantum = 1;

    if (!read_processes(p, &n, 1))
        return;

    if (alg == 6) {
        printf("Round Robin Time Quantum: ");
        if (scanf("%d", &quantum) != 1 || quantum <= 0) {
            printf("Time quantum must be greater than zero.\n");
            return;
        }
    }

    ScheduleResult result;

    if (!run_algorithm(p, n, alg, quantum, &result, 1)) {
        printf("Unable to allocate memory for the Gantt chart.\n");
        return;
    }

    free_result(&result);
}

static void performance_comparison(void)
{
    SchedProcess p[MAX_PROCESSES];
    ScheduleResult results[6] = {{0}};
    int n, quantum;

    printf("\nPerformance Comparison\n");
    printf("Enter process details for all six algorithms.\n");

    if (!read_processes(p, &n, 1))
        return;

    printf("Round Robin Time Quantum: ");
    if (scanf("%d", &quantum) != 1 || quantum <= 0) {
        printf("Time quantum must be greater than zero.\n");
        return;
    }

    for (int alg = 1; alg <= 6; alg++) {
        if (!run_algorithm(p, n, alg, quantum, &results[alg - 1], 0)) {
            printf("Unable to calculate performance comparison.\n");
            for (int j = 0; j < 6; j++)
                free_result(&results[j]);
            return;
        }
    }

    printf("\nRound Robin time quantum: %d\n", quantum);
    printf("\nPERFORMANCE COMPARISON\n");
    printf("%-28s %10s %10s %10s %12s\n",
           "Algorithm", "Avg WT", "Avg TAT", "Avg RT", "CPU Usage");

    const char *names[] = {
        "FCFS", "SJF Non-Preemptive", "SJF Preemptive",
        "Priority Non-Preemptive", "Priority Preemptive", "Round Robin"
    };

    for (int i = 0; i < 6; i++) {
        printf("%-28s %10.2f %10.2f %10.2f %11.2f%%\n",
               names[i], results[i].avg_wt, results[i].avg_tat,
               results[i].avg_rt, results[i].cpu_util);
        free_result(&results[i]);
    }
}

int main(void)
{
    int choice;

    do {
        printf("\nScheduling Algorithms\n");
        printf("1. FCFS\n");
        printf("2. SJF Non-Preemptive\n");
        printf("3. SJF Preemptive\n");
        printf("4. Priority Non-Preemptive\n");
        printf("5. Priority Preemptive\n");
        printf("6. Round Robin\n");
        printf("7. Performance Comparison\n");
        printf("0. Exit\n");
        printf("Choice: ");

        if (scanf("%d", &choice) != 1) {
            printf("Invalid input.\n");
            return 1;
        }

        switch (choice) {
            case 1:
            case 2:
            case 3:
            case 4:
            case 5:
            case 6:
                run_individual(choice);
                break;
            case 7:
                performance_comparison();
                break;
            case 0:
                break;
            default:
                printf("Invalid choice.\n");
        }
    } while (choice != 0);

    return 0;
}