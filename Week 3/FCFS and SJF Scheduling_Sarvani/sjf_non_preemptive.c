
#include <stdio.h>

#define MAX 100

typedef struct {
    int pid, at, bt, ct, tat, wt, rt;
} Process;

int main(void) {
    Process p[MAX];
    int done[MAX] = {0};
    int n, completed = 0, time = 0;
    double totalWT = 0, totalTAT = 0, totalRT = 0;

    printf("SJF Non-preemptive Scheduling\n");
    printf("Enter number of processes: ");

    if (scanf("%d", &n) != 1 || n < 1 || n > MAX) {
        printf("Invalid number of processes.\n");
        return 1;
    }

    for (int i = 0; i < n; i++) {
       
printf("\nEnter process ID for process %d: ", i + 1);
if (scanf("%d", &p[i].pid) != 1) {
    printf("Invalid process ID.\n");
    return 1;
}

printf("Enter AT for process %d: ", i + 1);
if (scanf("%d", &p[i].at) != 1 || p[i].at < 0) {
    printf("Invalid arrival time.\n");
    return 1;
}

printf("Enter BT for process %d: ", i + 1);
if (scanf("%d", &p[i].bt) != 1 || p[i].bt <= 0) {
    printf("Invalid burst time.\n");
    return 1;
}

    }

    printf("\nGantt Chart:\n");

    while (completed < n) {
        int best = -1;

        for (int i = 0; i < n; i++) {
            if (!done[i] && p[i].at <= time &&
                (best == -1 || p[i].bt < p[best].bt ||
                (p[i].bt == p[best].bt &&
                 p[i].at < p[best].at))) {
                best = i;
            }
        }

        if (best == -1) {
            printf("| IDLE ");
            int next = -1;

            for (int i = 0; i < n; i++) {
                if (!done[i] &&
                    (next == -1 || p[i].at < p[next].at))
                    next = i;
            }

            time = p[next].at;
            continue;
        }

        p[best].rt = time - p[best].at;
        printf("| P%d ", p[best].pid);

        time += p[best].bt;
        p[best].ct = time;
        p[best].tat = p[best].ct - p[best].at;
        p[best].wt = p[best].tat - p[best].bt;

        done[best] = 1;
        completed++;
    }

    printf("|\n");
    printf("\nPID\tAT\tBT\tCT\tTAT\tWT\tRT\n");

    for (int i = 0; i < n; i++) {
        printf("P%d\t%d\t%d\t%d\t%d\t%d\t%d\n",
               p[i].pid, p[i].at, p[i].bt, p[i].ct,
               p[i].tat, p[i].wt, p[i].rt);

        totalWT += p[i].wt;
        totalTAT += p[i].tat;
        totalRT += p[i].rt;
    }

    printf("\nAverage Waiting Time: %.2f\n", totalWT / n);
    printf("Average Turnaround Time: %.2f\n", totalTAT / n);
    printf("Average Response Time: %.2f\n", totalRT / n);

    return 0;
}
