
#include <stdio.h>

#define MAX 100

typedef struct {
    int pid, at, bt, ct, tat, wt, rt;
} Process;

int main(void) {
    Process p[MAX], temp;
    int n, i, j, time = 0;
    float avgWT = 0, avgTAT = 0, avgRT = 0;

    printf("Enter number of processes: ");
    scanf("%d", &n);

    if (n <= 0 || n > MAX) {
        printf("Invalid number of processes.\n");
        return 1;
    }

    for (i = 0; i < n; i++) {
       
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

        for (j = 0; j < i; j++) {
            if (p[j].pid == p[i].pid) {
                printf("Process IDs must be unique.\n");
                return 1;
            }
        }
    }

    /* Sort by arrival time; preserve input order for ties. */
    for (i = 0; i < n - 1; i++) {
        for (j = 0; j < n - i - 1; j++) {
            if (p[j].at > p[j + 1].at) {
                temp = p[j];
                p[j] = p[j + 1];
                p[j + 1] = temp;
            }
        }
    }

    printf("\nGantt chart:\n");
    for (i = 0; i < n; i++) {
        if (time < p[i].at) {
            printf("%d -- IDLE -- ", time);
            time = p[i].at;
        }

        p[i].rt = time - p[i].at;
        time += p[i].bt;
        p[i].ct = time;
        p[i].tat = p[i].ct - p[i].at;
        p[i].wt = p[i].tat - p[i].bt;

        printf("%d -- P%d -- ", time - p[i].bt, p[i].pid);
        avgWT += p[i].wt;
        avgTAT += p[i].tat;
        avgRT += p[i].rt;
    }
    printf("%d\n", time);

    printf("\nPID\tAT\tBT\tCT\tTAT\tWT\tRT\n");
    for (i = 0; i < n; i++) {
        printf("P%d\t%d\t%d\t%d\t%d\t%d\t%d\n",
               p[i].pid, p[i].at, p[i].bt,
               p[i].ct, p[i].tat, p[i].wt, p[i].rt);
    }

    printf("\nAverage Waiting Time: %.2f\n", avgWT / n);
    printf("Average Turnaround Time: %.2f\n", avgTAT / n);
    printf("Average Response Time: %.2f\n", avgRT / n);

    return 0;
}