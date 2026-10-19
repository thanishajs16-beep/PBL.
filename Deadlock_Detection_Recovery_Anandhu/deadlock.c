#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAXP 20
#define MAXR 20

int n, m;
int available[MAXR];
int allocation[MAXP][MAXR];
int request[MAXP][MAXR];
int terminated[MAXP];

int row_can_finish(int p, int work[]) {
    for (int j = 0; j < m; j++)
        if (request[p][j] > work[j]) return 0;
    return 1;
}

void detect_deadlock(void) {
    int work[MAXR], finish[MAXP], deadlocked[MAXP], count = 0;
    for (int j = 0; j < m; j++) work[j] = available[j];

    for (int i = 0; i < n; i++) {
        int has_allocation = 0;
        for (int j = 0; j < m; j++)
            if (allocation[i][j] > 0) has_allocation = 1;
        finish[i] = terminated[i] || !has_allocation;
    }

    int changed;
    do {
        changed = 0;
        for (int i = 0; i < n; i++) {
            if (!finish[i] && row_can_finish(i, work)) {
                for (int j = 0; j < m; j++) work[j] += allocation[i][j];
                finish[i] = 1;
                changed = 1;
            }
        }
    } while (changed);

    printf("DETECTION\n");
    for (int i = 0; i < n; i++) {
        deadlocked[i] = !finish[i] && !terminated[i];
        if (deadlocked[i]) count++;
    }

    if (count == 0) {
        printf("STATUS: NO_DEADLOCK\n");
        printf("DEADLOCKED:");
    } else {
        printf("STATUS: DEADLOCK\n");
        printf("DEADLOCKED:");
        for (int i = 0; i < n; i++)
            if (deadlocked[i]) printf(" P%d", i + 1);
    }
    printf("\n");
}

void print_rag(void) {
    printf("RAG\n");
    printf("ALLOC_EDGES:\n");
    for (int i = 0; i < n; i++)
        for (int j = 0; j < m; j++)
            if (allocation[i][j] > 0)
                printf("R%d -> P%d (units=%d)\n", j + 1, i + 1, allocation[i][j]);
    printf("REQUEST_EDGES:\n");
    for (int i = 0; i < n; i++)
        if (!terminated[i])
            for (int j = 0; j < m; j++)
                if (request[i][j] > 0)
                    printf("P%d -> R%d (units=%d)\n", i + 1, j + 1, request[i][j]);
}

int main(void) {
    char operation[20];
    int selected = 0;

    if (scanf("%d %d", &n, &m) != 2 || n < 1 || n > MAXP || m < 1 || m > MAXR) {
        fprintf(stderr, "ERROR: Processes/resources must be between 1 and 20.\n");
        return 1;
    }
    if (scanf("%19s %d", operation, &selected) != 2) {
        fprintf(stderr, "ERROR: Missing operation and selected process.\n");
        return 1;
    }

    for (int j = 0; j < m; j++)
        if (scanf("%d", &available[j]) != 1 || available[j] < 0) {
            fprintf(stderr, "ERROR: Available values must be non-negative integers.\n");
            return 1;
        }

    for (int i = 0; i < n; i++)
        for (int j = 0; j < m; j++)
            if (scanf("%d", &allocation[i][j]) != 1 || allocation[i][j] < 0) {
                fprintf(stderr, "ERROR: Allocation values must be non-negative integers.\n");
                return 1;
            }

    for (int i = 0; i < n; i++)
        for (int j = 0; j < m; j++)
            if (scanf("%d", &request[i][j]) != 1 || request[i][j] < 0) {
                fprintf(stderr, "ERROR: Request values must be non-negative integers.\n");
                return 1;
            }

    if (strcmp(operation, "recover") == 0) {
        if (selected < 1 || selected > n) {
            fprintf(stderr, "ERROR: Select a valid process number.\n");
            return 1;
        }
        int p = selected - 1;
        if (terminated[p]) {
            fprintf(stderr, "ERROR: Process already terminated.\n");
            return 1;
        }
        for (int j = 0; j < m; j++) {
            available[j] += allocation[p][j];
            allocation[p][j] = 0;
            request[p][j] = 0;
        }
        terminated[p] = 1;
        printf("RECOVERY: Terminated P%d and released its allocated resources.\n", selected);
        printf("UPDATED_AVAILABLE:");
        for (int j = 0; j < m; j++) printf(" %d", available[j]);
        printf("\n");
        detect_deadlock();
        print_rag();
    } else if (strcmp(operation, "graph") == 0) {
        print_rag();
    } else if (strcmp(operation, "detect") == 0) {
        detect_deadlock();
    } else {
        fprintf(stderr, "ERROR: Operation must be detect, graph, or recover.\n");
        return 1;
    }
    return 0;
}
