
#include <stdio.h>

#define MAX 100

typedef struct {
    int pid, at, bt, ct, tat, wt, rt;
} Process;

int main(void) {
    Process p[MAX];
    int remaining[MAX], firstStart[MAX];
    int n, completed = 0, time = 0;
    double totalWT = 0, totalTAT = 0, totalRT = 0;

    printf("SJF Preemptive (SRTF) Scheduling\n");
    printf("Enter number of processes: ");

    if (scanf("%d", &n) != 1 || n < 1 || n > MAX) {
        printf("Invalid number of processes.\n");
        return 1;
    }

    for (int i = 0; i < n; i++) {
        p[i].pid = i + 1;

        printf("Enter arrival time and burst time for P%d: ",
               p[i].pid);

        if (scanf("%d %d", &p[i].at, &p[i].bt) != 2 ||
            p[i].at < 0 || p[i].bt <= 0) {
            printf("Invalid input.\n");
            return 1;
        }

        remaining[i] = p[i].bt;
        firstStart[i] = -1;
    }

    printf("\nGantt Chart:\n");

    while (completed < n) {
        int best = -1;

        for (int i = 0; i < n; i++) {
            if (remaining[i] > 0 && p[i].at <= time &&
                (best == -1 ||
                 remaining[i] < remaining[best] ||
                (remaining[i] == remaining[best] &&
                 p[i].at < p[best].at))) {
                best = i;
            }
        }

        if (best == -1) {
            printf("| IDLE ");
            time++;
            continue;
        }

        if (firstStart[best] == -1) {
            firstStart[best] = time;
            p[best].rt = time - p[best].at;
        }

        printf("| P%d ", p[best].pid);
        remaining[best]--;
        time++;

        if (remaining[best] == 0) {
            p[best].ct = time;
            completed++;
        }
    }

    printf("|\n");
    printf("\nPID\tAT\tBT\tCT\tTAT\tWT\tRT\n");

    for (int i = 0; i < n; i++) {
        p[i].tat = p[i].ct - p[i].at;
        p[i].wt = p[i].tat - p[i].bt;

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
