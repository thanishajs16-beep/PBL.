
#include <stdio.h>
#define MAX 100

typedef struct {
    int id, priority, arrived;
    long long at, bt, rem, ct, rt;
} Process;

void add_arrivals(Process p[], int n, int q[], int *rear,
                  int *count, long long time)
{
    while (1) {
        int b = -1;
        for (int i = 0; i < n; i++)
            if (!p[i].arrived && p[i].at <= time &&
                (b == -1 || p[i].at < p[b].at))
                b = i;

        if (b == -1) break;
        q[*rear] = b;
        *rear = (*rear + 1) % n;
        (*count)++;
        p[b].arrived = 1;
    }
}

int main(void)
{
    Process p[MAX] = {0};
    int q[MAX], front = 0, rear = 0, count = 0;
    int n, done = 0;
    long long time = 0, quantum, busy = 0;
    double wt = 0, tat = 0, rt = 0;

    printf("Number of processes (1-100): ");
    if (scanf("%d", &n) != 1 || n < 1 || n > MAX) {
        printf("Invalid number of processes.\n");
        return 1;
    }

    for (int i = 0; i < n; i++) {
        printf("\nEnter Process ID for process %d: ", i + 1);
        if (scanf("%d", &p[i].id) != 1) return 1;

        printf("Enter Arrival Time: ");
        if (scanf("%lld", &p[i].at) != 1) return 1;

        printf("Enter Burst Time: ");
        if (scanf("%lld", &p[i].bt) != 1) return 1;

        printf("Enter Priority: ");
        if (scanf("%d", &p[i].priority) != 1) return 1;

        if (p[i].id <= 0 || p[i].at < 0 || p[i].bt <= 0) {
            printf("Invalid process details.\n");
            return 1;
        }

        for (int j = 0; j < i; j++)
            if (p[i].id == p[j].id) {
                printf("Process IDs must be unique.\n");
                return 1;
            }

        p[i].rem = p[i].bt;
        p[i].rt = -1;
        busy += p[i].bt;
    }

    printf("Enter Time Quantum: ");
    if (scanf("%lld", &quantum) != 1 || quantum <= 0) {
        printf("Invalid time quantum.\n");
        return 1;
    }

    printf("\nGantt chart:\n0");

    while (done < n) {
        if (count == 0) {
            int b = -1;
            for (int i = 0; i < n; i++)
                if (!p[i].arrived &&
                    (b == -1 || p[i].at < p[b].at))
                    b = i;

            if (b == -1) break;

            if (p[b].at > time) {
                time = p[b].at;
                printf(" -- IDLE -- %lld", time);
            }

            add_arrivals(p, n, q, &rear, &count, time);
            continue;
        }

        int i = q[front];
        front = (front + 1) % n;
        count--;

        if (p[i].rt == -1)
            p[i].rt = time - p[i].at;

        long long run = p[i].rem < quantum ? p[i].rem : quantum;
        time += run;
        p[i].rem -= run;

        printf(" -- P%d -- %lld", p[i].id, time);

        add_arrivals(p, n, q, &rear, &count, time);

        if (p[i].rem > 0) {
            q[rear] = i;
            rear = (rear + 1) % n;
            count++;
        } else {
            p[i].ct = time;
            done++;
        }
    }

    printf("\n\nPID\tAT\tBT\tPriority\tCT\tWT\tTAT\tRT\n");

    for (int i = 0; i < n; i++) {
        long long t = p[i].ct - p[i].at;
        long long w = t - p[i].bt;

        printf("P%d\t%lld\t%lld\t%d\t\t%lld\t%lld\t%lld\t%lld\n",
               p[i].id, p[i].at, p[i].bt, p[i].priority,
               p[i].ct, w, t, p[i].rt);

        wt += w;
        tat += t;
        rt += p[i].rt;
    }

    printf("\nAverage Waiting Time: %.2f\n", wt / n);
    printf("Average Turnaround Time: %.2f\n", tat / n);
    printf("Average Response Time: %.2f\n", rt / n);
    printf("Total elapsed time: %lld\n", time);
    printf("CPU busy time: %lld\n", busy);
    printf("CPU idle time: %lld\n", time - busy);
    printf("CPU Utilization: %.2f%%\n", (double)busy / time * 100);

    return 0;
}