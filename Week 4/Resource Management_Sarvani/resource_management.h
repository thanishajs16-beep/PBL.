
#ifndef RESOURCE_MANAGEMENT_H
#define RESOURCE_MANAGEMENT_H

#define MAX_PROCESSES 20
#define MAX_RESOURCES 10
#define NAME_LENGTH 32

typedef struct {
    int processCount;
    int resourceCount;

    char processNames[MAX_PROCESSES][NAME_LENGTH];
    char resourceNames[MAX_RESOURCES][NAME_LENGTH];

    int total[MAX_RESOURCES];
    int available[MAX_RESOURCES];

    int allocation[MAX_PROCESSES][MAX_RESOURCES];
    int maximum[MAX_PROCESSES][MAX_RESOURCES];
    int need[MAX_PROCESSES][MAX_RESOURCES];
} ResourceManager;

void initManager(ResourceManager *rm);

int addResource(ResourceManager *rm, const char *name,
                int totalInstances);

int addProcess(ResourceManager *rm, const char *name);

int setAllocation(ResourceManager *rm, int p, int r, int value);

int setMaximum(ResourceManager *rm, int p, int r, int value);

void calculateNeed(ResourceManager *rm);

void displayMatrices(ResourceManager *rm);

int runBankers(ResourceManager *rm, int sequence[]);

int requestResources(ResourceManager *rm, int p,
                     int request[], int sequence[]);

#endif
