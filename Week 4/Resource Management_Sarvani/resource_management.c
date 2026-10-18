
#include <stdio.h>
#include <string.h>
#include "resource_management.h"

void initManager(ResourceManager *rm) {
    memset(rm, 0, sizeof(ResourceManager));
}

int addResource(ResourceManager *rm, const char *name,
                int totalInstances) {
    if (rm->resourceCount >= MAX_RESOURCES ||
        totalInstances < 0 ||
        name == NULL || strlen(name) >= NAME_LENGTH)
        return 0;

    int r = rm->resourceCount;

    strcpy(rm->resourceNames[r], name);
    rm->total[r] = totalInstances;
    rm->available[r] = totalInstances;

    rm->resourceCount++;
    return 1;
}

int addProcess(ResourceManager *rm, const char *name) {
    if (rm->processCount >= MAX_PROCESSES ||
        name == NULL || strlen(name) >= NAME_LENGTH)
        return 0;

    strcpy(rm->processNames[rm->processCount], name);
    rm->processCount++;

    return 1;
}

int setAllocation(ResourceManager *rm, int p, int r, int value) {
    if (p < 0 || p >= rm->processCount ||
        r < 0 || r >= rm->resourceCount ||
        value < 0 || value > rm->maximum[p][r])
        return 0;

    int old = rm->allocation[p][r];
    rm->allocation[p][r] = value;

    // Recalculate available resources.
    for (int j = 0; j < rm->resourceCount; j++) {
        int used = 0;

        for (int i = 0; i < rm->processCount; i++)
            used += rm->allocation[i][j];

        if (used > rm->total[j]) {
            rm->allocation[p][r] = old;
            return 0;
        }

        rm->available[j] = rm->total[j] - used;
    }

    calculateNeed(rm);
    return 1;
}

int setMaximum(ResourceManager *rm, int p, int r, int value) {
    if (p < 0 || p >= rm->processCount ||
        r < 0 || r >= rm->resourceCount ||
        value < rm->allocation[p][r])
        return 0;

    rm->maximum[p][r] = value;
    calculateNeed(rm);

    return 1;
}

void calculateNeed(ResourceManager *rm) {
    for (int i = 0; i < rm->processCount; i++) {
        for (int j = 0; j < rm->resourceCount; j++) {
            rm->need[i][j] =
                rm->maximum[i][j] - rm->allocation[i][j];
        }
    }
}

void displayMatrices(ResourceManager *rm) {
    printf("\nAvailable: ");
    for (int j = 0; j < rm->resourceCount; j++)
        printf("%s=%d ", rm->resourceNames[j], rm->available[j]);

    printf("\n\nAllocation Matrix\n");
    printf("PID\t");
    for (int j = 0; j < rm->resourceCount; j++)
        printf("%s\t", rm->resourceNames[j]);
    printf("\n");

    for (int i = 0; i < rm->processCount; i++) {
        printf("%s\t", rm->processNames[i]);
        for (int j = 0; j < rm->resourceCount; j++)
            printf("%d\t", rm->allocation[i][j]);
        printf("\n");
    }

    printf("\nMaximum Matrix\nPID\t");
    for (int j = 0; j < rm->resourceCount; j++)
        printf("%s\t", rm->resourceNames[j]);
    printf("\n");

    for (int i = 0; i < rm->processCount; i++) {
        printf("%s\t", rm->processNames[i]);
        for (int j = 0; j < rm->resourceCount; j++)
            printf("%d\t", rm->maximum[i][j]);
        printf("\n");
    }

    calculateNeed(rm);

    printf("\nNeed Matrix\nPID\t");
    for (int j = 0; j < rm->resourceCount; j++)
        printf("%s\t", rm->resourceNames[j]);
    printf("\n");

    for (int i = 0; i < rm->processCount; i++) {
        printf("%s\t", rm->processNames[i]);
        for (int j = 0; j < rm->resourceCount; j++)
            printf("%d\t", rm->need[i][j]);
        printf("\n");
    }
}

// Returns 1 for SAFE, 0 for UNSAFE or invalid state.
int runBankers(ResourceManager *rm, int sequence[]) {
    int work[MAX_RESOURCES];
    int finish[MAX_PROCESSES] = {0};
    int count = 0;

    if (rm->processCount == 0 || rm->resourceCount == 0)
        return 0;

    calculateNeed(rm);

    for (int j = 0; j < rm->resourceCount; j++)
        work[j] = rm->available[j];

    while (count < rm->processCount) {
        int found = 0;

        for (int i = 0; i < rm->processCount; i++) {
            if (finish[i])
                continue;

            int canFinish = 1;

            for (int j = 0; j < rm->resourceCount; j++) {
                if (rm->need[i][j] > work[j]) {
                    canFinish = 0;
                    break;
                }
            }

            if (canFinish) {
                for (int j = 0; j < rm->resourceCount; j++)
                    work[j] += rm->allocation[i][j];

                finish[i] = 1;
                sequence[count++] = i;
                found = 1;
            }
        }

        if (!found)
            return 0;
    }

    return 1;
}

// Grant a request only if the resulting state is safe.
int requestResources(ResourceManager *rm, int p,
                     int request[], int sequence[]) {
    if (p < 0 || p >= rm->processCount)
        return 0;

    calculateNeed(rm);

    for (int j = 0; j < rm->resourceCount; j++) {
        if (request[j] < 0 ||
            request[j] > rm->need[p][j] ||
            request[j] > rm->available[j])
            return 0;
    }

    // Temporarily allocate the requested resources.
    for (int j = 0; j < rm->resourceCount; j++) {
        rm->available[j] -= request[j];
        rm->allocation[p][j] += request[j];
    }

    calculateNeed(rm);

    if (runBankers(rm, sequence))
        return 1;

    // Unsafe: roll back the temporary allocation.
    for (int j = 0; j < rm->resourceCount; j++) {
        rm->available[j] += request[j];
        rm->allocation[p][j] -= request[j];
    }

    calculateNeed(rm);
    return 0;
}
