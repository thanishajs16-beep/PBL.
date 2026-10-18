
#include <stdio.h>

#define MAX_PROCESSES 10
#define MAX_RESOURCES 10

int process_count = 0;
int resource_count = 0;

int pid[MAX_PROCESSES];
int total[MAX_RESOURCES];
int available[MAX_RESOURCES];

int allocation[MAX_PROCESSES][MAX_RESOURCES] = {0};
int maximum[MAX_PROCESSES][MAX_RESOURCES] = {0};
int need[MAX_PROCESSES][MAX_RESOURCES] = {0};

int findProcess(int process_id)
{
    for (int i = 0; i < process_count; i++)
        if (pid[i] == process_id)
            return i;

    return -1;
}

void calculateNeed(void)
{
    for (int i = 0; i < process_count; i++)
        for (int j = 0; j < resource_count; j++)
            need[i][j] = maximum[i][j] - allocation[i][j];
}

int updateAvailable(void)
{
    for (int j = 0; j < resource_count; j++)
    {
        int used = 0;

        for (int i = 0; i < process_count; i++)
            used += allocation[i][j];

        if (used > total[j])
            return 0;

        available[j] = total[j] - used;
    }

    return 1;
}

void createProcess(void)
{
    int new_pid;

    if (process_count >= MAX_PROCESSES)
    {
        printf("Maximum process limit reached.\n");
        return;
    }

    printf("Enter new process ID: ");

    if (scanf("%d", &new_pid) != 1)
    {
        printf("Invalid input.\n");
        return;
    }

    if (findProcess(new_pid) != -1)
    {
        printf("Error: PID already exists.\n");
        return;
    }

    pid[process_count] = new_pid;

    for (int j = 0; j < resource_count; j++)
    {
        allocation[process_count][j] = 0;
        maximum[process_count][j] = 0;
        need[process_count][j] = 0;
    }

    process_count++;

    printf("Process P%d created successfully.\n", new_pid);
}

void createResource(void)
{
    int instances;

    if (resource_count >= MAX_RESOURCES)
    {
        printf("Maximum resource-type limit reached.\n");
        return;
    }

    printf("Enter total instances for resource R%d: ",
           resource_count);

    if (scanf("%d", &instances) != 1 || instances < 0)
    {
        printf("Invalid number of instances.\n");
        return;
    }

    total[resource_count] = instances;
    available[resource_count] = instances;

    for (int i = 0; i < process_count; i++)
    {
        allocation[i][resource_count] = 0;
        maximum[i][resource_count] = 0;
        need[i][resource_count] = 0;
    }

    resource_count++;

    printf("Resource R%d created successfully.\n",
           resource_count - 1);
}

void setAllocation(void)
{
    int process_id, resource_id, value;

    if (process_count == 0 || resource_count == 0)
    {
        printf("Create at least one process and resource first.\n");
        return;
    }

    printf("Enter process ID: ");
    if (scanf("%d", &process_id) != 1)
        return;

    int p = findProcess(process_id);

    if (p == -1)
    {
        printf("Process not found.\n");
        return;
    }

    printf("Enter resource ID (0 to %d): ", resource_count - 1);
    if (scanf("%d", &resource_id) != 1 ||
        resource_id < 0 || resource_id >= resource_count)
    {
        printf("Invalid resource ID.\n");
        return;
    }

    printf("Enter allocated instances: ");
    if (scanf("%d", &value) != 1 || value < 0 ||
        value > maximum[p][resource_id])
    {
        printf("Invalid allocation. It cannot exceed maximum.\n");
        return;
    }

    int old_value = allocation[p][resource_id];
    allocation[p][resource_id] = value;

    if (!updateAvailable())
    {
        allocation[p][resource_id] = old_value;
        updateAvailable();
        printf("Allocation rejected: total resources exceeded.\n");
        return;
    }

    calculateNeed();
    printf("Allocation updated successfully.\n");
}

void setMaximum(void)
{
    int process_id, resource_id, value;

    if (process_count == 0 || resource_count == 0)
    {
        printf("Create at least one process and resource first.\n");
        return;
    }

    printf("Enter process ID: ");
    if (scanf("%d", &process_id) != 1)
        return;

    int p = findProcess(process_id);

    if (p == -1)
    {
        printf("Process not found.\n");
        return;
    }

    printf("Enter resource ID (0 to %d): ", resource_count - 1);
    if (scanf("%d", &resource_id) != 1 ||
        resource_id < 0 || resource_id >= resource_count)
    {
        printf("Invalid resource ID.\n");
        return;
    }

    printf("Enter maximum required instances: ");
    if (scanf("%d", &value) != 1 ||
        value < allocation[p][resource_id])
    {
        printf("Maximum cannot be less than current allocation.\n");
        return;
    }

    maximum[p][resource_id] = value;
    calculateNeed();

    printf("Maximum requirement updated successfully.\n");
}

int isSafe(int sequence[])
{
    int work[MAX_RESOURCES];
    int finish[MAX_PROCESSES] = {0};
    int count = 0;

    calculateNeed();

    for (int j = 0; j < resource_count; j++)
        work[j] = available[j];

    while (count < process_count)
    {
        int found = 0;

        for (int i = 0; i < process_count; i++)
        {
            if (finish[i])
                continue;

            int possible = 1;

            for (int j = 0; j < resource_count; j++)
            {
                if (need[i][j] > work[j])
                {
                    possible = 0;
                    break;
                }
            }

            if (possible)
            {
                for (int j = 0; j < resource_count; j++)
                    work[j] += allocation[i][j];

                sequence[count++] = i;
                finish[i] = 1;
                found = 1;
            }
        }

        if (!found)
            return 0;
    }

    return 1;
}

void displayVector(const char *title, const int values[])
{
    printf("\n%s: ", title);

    for (int j = 0; j < resource_count; j++)
        printf("R%d=%d  ", j, values[j]);

    printf("\n");
}

void displayMatrix(const char *title,
                   int matrix[MAX_PROCESSES][MAX_RESOURCES])
{
    printf("\n%s\nPID\t", title);

    for (int j = 0; j < resource_count; j++)
        printf("R%d\t", j);

    printf("\n");

    for (int i = 0; i < process_count; i++)
    {
        printf("P%d\t", pid[i]);

        for (int j = 0; j < resource_count; j++)
            printf("%d\t", matrix[i][j]);

        printf("\n");
    }
}

void displayAll(void)
{
    if (process_count == 0 || resource_count == 0)
    {
        printf("Create at least one process and resource first.\n");
        return;
    }

    updateAvailable();
    calculateNeed();

    printf("\nProcesses: %d | Resource types: %d\n",
           process_count, resource_count);

    displayVector("Total resources", total);
    displayVector("Available resources", available);
    displayMatrix("Allocation Matrix", allocation);
    displayMatrix("Maximum Matrix", maximum);
    displayMatrix("Need Matrix", need);
}

void displayStatus(void)
{
    int sequence[MAX_PROCESSES];

    if (process_count == 0 || resource_count == 0)
    {
        printf("Create processes and resources first.\n");
        return;
    }

    if (!updateAvailable())
    {
        printf("Invalid state: allocations exceed total resources.\n");
        return;
    }

    if (isSafe(sequence))
    {
        printf("\nSystem Status: SAFE\nSafe Sequence: ");

        for (int i = 0; i < process_count; i++)
            printf("P%d%s", pid[sequence[i]],
                   i == process_count - 1 ? "\n" : " -> ");
    }
    else
    {
        printf("\nSystem Status: UNSAFE\n");
        printf("No safe sequence exists for the current state.\n");
    }
}

void requestResources(void)
{
    int process_id, request[MAX_RESOURCES];
    int sequence[MAX_PROCESSES];

    if (process_count == 0 || resource_count == 0)
    {
        printf("Create processes and resources first.\n");
        return;
    }

    printf("Enter process ID: ");
    if (scanf("%d", &process_id) != 1)
        return;

    int p = findProcess(process_id);

    if (p == -1)
    {
        printf("Process not found.\n");
        return;
    }

    for (int j = 0; j < resource_count; j++)
    {
        printf("Request for R%d: ", j);

        if (scanf("%d", &request[j]) != 1 || request[j] < 0)
        {
            printf("Invalid request.\n");
            return;
        }

        if (request[j] > need[p][j])
        {
            printf("Request exceeds remaining need.\n");
            return;
        }

        if (request[j] > available[j])
        {
            printf("Insufficient available resources.\n");
            return;
        }
    }

    for (int j = 0; j < resource_count; j++)
    {
        available[j] -= request[j];
        allocation[p][j] += request[j];
        need[p][j] -= request[j];
    }

    if (isSafe(sequence))
    {
        printf("Request GRANTED. System remains SAFE.\n");
        printf("Safe Sequence: ");

        for (int i = 0; i < process_count; i++)
            printf("P%d%s", pid[sequence[i]],
                   i == process_count - 1 ? "\n" : " -> ");
    }
    else
    {
        for (int j = 0; j < resource_count; j++)
        {
            available[j] += request[j];
            allocation[p][j] -= request[j];
            need[p][j] += request[j];
        }

        printf("Request DENIED. System would become UNSAFE.\n");
    }
}

int main(void)
{
    int choice;

    do
    {
        printf("\n===== RESOURCE MANAGEMENT MENU =====\n");
        printf("1. Create Process\n");
        printf("2. Create Resource\n");
        printf("3. Set Allocation\n");
        printf("4. Set Maximum\n");
        printf("5. Display All Matrices\n");
        printf("6. Display Need Matrix\n");
        printf("7. Run Banker's Algorithm\n");
        printf("8. Request Resources\n");
        printf("9. Display System Status\n");
        printf("10. Exit\n");
        printf("Enter choice: ");

        if (scanf("%d", &choice) != 1)
        {
            printf("Invalid input. Restart the program.\n");
            return 1;
        }

        switch (choice)
        {
            case 1: createProcess(); break;
            case 2: createResource(); break;
            case 3: setAllocation(); break;
            case 4: setMaximum(); break;
            case 5: displayAll(); break;
            case 6:
                calculateNeed();
                displayMatrix("Need Matrix", need);
                break;
            case 7: displayStatus(); break;
            case 8: requestResources(); break;
            case 9: displayStatus(); break;
            case 10: printf("Exiting program.\n"); break;
            default: printf("Invalid choice.\n");
        }

    } while (choice != 10);

    return 0;
}
