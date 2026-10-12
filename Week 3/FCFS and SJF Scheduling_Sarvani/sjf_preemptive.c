
#include <stdio.h>
#include<string.h>
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
       
int unique;

do {
    unique = 1;

       printf("Enter process ID for process %d: ", i + 1);
    scanf("%d", &p[i].pid);

    for (int j = 0; j < i; j++) {
        if (p[i].pid == p[j].pid) {
            printf("Error: Process ID must be unique. Try again.\n");
            unique = 0;
            break;
        }
    }
} while (!unique);

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
        remaining[i] = p[i].bt;
        firstStart[i] = -1;
    }

    printf("\nGantt Chart:\n");
      int lastProcess = -1;
      int segmentStart = time;
    
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
        if (lastProcess != -2) {
            if (lastProcess != -1)
                printf("%d -- ", time);

            if (best == -1) {
    if (lastProcess != -2) {
        if (lastProcess != -1)
            printf("%d -- ", time);

        printf("IDLE -- ");
        lastProcess = -2;
    }

    time++;
    continue;
}
            lastProcess = -2;
        }
        time++;
        continue;
    }

    if (firstStart[best] == -1) {
        firstStart[best] = time;
        p[best].rt = time - p[best].at;
    }

    if (lastProcess != best) {
        if (lastProcess != -1)
            printf("%d -- ", time);

        if (lastProcess != best) {
    if (lastProcess != -1)
        printf("%d -- ", time);

    printf("P%d -- ", p[best].pid);
    lastProcess = best;
}
        lastProcess = best;
    }

    remaining[best]--;
    time++;

    if (remaining[best] == 0) {
        p[best].ct = time;
        completed++;
    }
}
      printf("%d\n", time);
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
