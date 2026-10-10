
#include <stdio.h>

#define MAX 100

typedef struct {
    int pid, at, bt, ct, tat, wt, rt;
} Process;

int main(void) {
    Process p[MAX], temp;
    int n, time = 0;
    double totalWT = 0, totalTAT = 0, totalRT = 0;

    printf("FCFS Scheduling\n");
    printf("Enter number of processes: ");
    scanf("%d", &n);

    if (n < 1 || n > MAX) {
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
    }

    /* Sort by arrival time */
    for (int i = 0; i < n - 1; i++) {
        for (int j = 0; j < n - i - 1; j++) {
            if (p[j].at > p[j + 1].at) {
                temp = p[j];
                p[j] = p[j + 1];
                p[j + 1] = temp;
            }
        }
    }

    printf("\nGantt Chart:\n");
    for (int i = 0; i < n; i++) {
        if (time < p[i].at) {
            printf("| IDLE ");
            time = p[i].at;
        }

        p[i].rt = time - p[i].at;
        printf("| P%d ", p[i].pid);
        time += p[i].bt;
        p[i].ct = time;
        p[i].tat = p[i].ct - p[i].at;
        p[i].wt = p[i].tat - p[i].bt;
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
