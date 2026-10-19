

#include <stdio.h>
#include <string.h>
#include "../deadlock.h"

/* Find a process by its PID in the shared system. */
static int find_process(const DeadlockSystem *system, int pid)
{
    for (int i = 0; i < system->process_count; i++)
    {
        if (system->pid[i] == pid)
            return i;
    }

    return -1;
}

static int read_int(const char *prompt, int *value)
{
    printf("%s", prompt);

    if (scanf("%d", value) != 1)
    {
        int ch;
        while ((ch = getchar()) != '\n' && ch != EOF)
            ;
        printf("Invalid input.\n");
        return 0;
    }

    return 1;
}

static void create_process(DeadlockSystem *system)
{
    int pid;

    if (!read_int("Enter new process ID: ", &pid))
        return;

    if (find_process(system, pid) != -1)
    {
        printf("Error: PID already exists.\n");
        return;
    }

    if (deadlock_add_process(system, pid))
        printf("Process P%d created successfully.\n", pid);
    else
        printf("Could not create process. Check the process limit.\n");
}

static void create_resource(DeadlockSystem *system)
{
    int total;

    if (!read_int("Enter total resource instances: ", &total))
        return;

    if (total < 0)
    {
        printf("Total instances cannot be negative.\n");
        return;
    }

    if (deadlock_add_resource(system, total))
        printf("Resource R%d created successfully.\n",
               system->resource_count - 1);
    else
        printf("Could not create resource. Check the resource limit.\n");
}

static int get_process_and_resource(
    DeadlockSystem *system, int *process_index, int *resource_index)
{
    int pid, rid;

    if (system->process_count == 0 || system->resource_count == 0)
    {
        printf("Create at least one process and resource first.\n");
        return 0;
    }

    if (!read_int("Enter process ID: ", &pid))
        return 0;

    *process_index = find_process(system, pid);

    if (*process_index < 0)
    {
        printf("Process not found.\n");
        return 0;
    }

    if (!read_int("Enter resource index (starting from 0): ", &rid))
        return 0;

    if (rid < 0 || rid >= system->resource_count)
    {
        printf("Invalid resource index.\n");
        return 0;
    }

    *resource_index = rid;
    return 1;
}

static void set_allocation(DeadlockSystem *system)
{
    int p, r, value;

    if (!get_process_and_resource(system, &p, &r))
        return;

    if (!read_int("Enter allocated instances: ", &value))
        return;

    if (value < 0 || value > system->maximum[p][r])
    {
        printf("Allocation must be non-negative and not exceed maximum.\n");
        return;
    }

    int old_value = system->allocation[p][r];

    if (!deadlock_set_allocation(system, p, r, value))
    {
        printf("Allocation rejected by the shared backend.\n");
        return;
    }

    if (!deadlock_recalculate_available(system) ||
        !deadlock_recalculate_need(system) ||
        !deadlock_validate_system(system))
    {
        deadlock_set_allocation(system, p, r, old_value);
        deadlock_recalculate_available(system);
        deadlock_recalculate_need(system);
        printf("Allocation rejected: it would make the data invalid.\n");
        return;
    }

    printf("Allocation updated successfully.\n");
}

static void set_maximum(DeadlockSystem *system)
{
    int p, r, value;

    if (!get_process_and_resource(system, &p, &r))
        return;

    if (!read_int("Enter maximum required instances: ", &value))
        return;

    if (value < system->allocation[p][r])
    {
        printf("Maximum cannot be less than current allocation.\n");
        return;
    }

    if (!deadlock_set_maximum(system, p, r, value) ||
        !deadlock_recalculate_need(system) ||
        !deadlock_validate_system(system))
    {
        printf("Maximum requirement rejected by the shared backend.\n");
        return;
    }

    printf("Maximum requirement updated successfully.\n");
}

static void display_vector(
    const char *title, const int vector[], int count)
{
    printf("\n%s: ", title);

    for (int r = 0; r < count; r++)
        printf("R%d=%d  ", r, vector[r]);

    printf("\n");
}

static void display_matrix(
    const char *title,
    const DeadlockSystem *system,
    const int matrix[DEADLOCK_MAX_PROCESSES][DEADLOCK_MAX_RESOURCES])
{
    printf("\n%s\nPID\t", title);

    for (int r = 0; r < system->resource_count; r++)
        printf("R%d\t", r);

    printf("\n");

    for (int p = 0; p < system->process_count; p++)
    {
        printf("P%d\t", system->pid[p]);

        for (int r = 0; r < system->resource_count; r++)
            printf("%d\t", matrix[p][r]);

        printf("\n");
    }
}

static void display_matrices(DeadlockSystem *system)
{
    if (!deadlock_recalculate_available(system) ||
        !deadlock_recalculate_need(system))
    {
        printf("Could not calculate matrices. Check system data.\n");
        return;
    }

    printf("\nProcesses: %d | Resource types: %d\n",
           system->process_count, system->resource_count);

    display_vector("Total", system->total, system->resource_count);
    display_vector("Available", system->available,
                   system->resource_count);

    display_matrix("Allocation Matrix", system, system->allocation);
    display_matrix("Maximum Matrix", system, system->maximum);
    display_matrix("Need Matrix", system, system->need);
    display_matrix("Request Matrix", system, system->request);
}

static void run_bankers(DeadlockSystem *system)
{
    BankerResult result = {0};

    DeadlockSafetyStatus status =
        run_bankers_algorithm(system, &result);

    if (status == DEADLOCK_SAFETY_SAFE)
    {
        printf("\nSystem Status: SAFE\nSafe Sequence: ");

        for (int i = 0; i < result.safe_sequence_length; i++)
            printf("P%d%s", result.safe_sequence_pids[i],
                   i == result.safe_sequence_length - 1
                       ? "\n" : " -> ");
    }
    else if (status == DEADLOCK_SAFETY_UNSAFE)
    {
        printf("\nSystem Status: UNSAFE\n");
    }
    else
    {
        printf("\nBanker's Algorithm could not validate the system.\n");
    }
}

static void submit_request(DeadlockSystem *system)
{
    int pid, p;
    int request[DEADLOCK_MAX_RESOURCES] = {0};
    BankerResult result = {0};

    if (system->process_count == 0 || system->resource_count == 0)
    {
        printf("Create processes and resources first.\n");
        return;
    }

    if (!read_int("Enter process ID: ", &pid))
        return;

    p = find_process(system, pid);

    if (p < 0)
    {
        printf("Process not found.\n");
        return;
    }

    for (int r = 0; r < system->resource_count; r++)
    {
        char prompt[80];
        snprintf(prompt, sizeof(prompt), "Request for R%d: ", r);

        if (!read_int(prompt, &request[r]) || request[r] < 0)
        {
            printf("Invalid request.\n");
            return;
        }
    }

    DeadlockRequestStatus status =
        request_resources(system, p, request, &result);

    switch (status)
    {
        case DEADLOCK_REQUEST_GRANTED:
            printf("Request GRANTED. System remains safe.\n");
            if (result.safe_sequence_length > 0)
            {
                printf("Safe Sequence: ");
                for (int i = 0; i < result.safe_sequence_length; i++)
                    printf("P%d%s", result.safe_sequence_pids[i],
                           i == result.safe_sequence_length - 1
                               ? "\n" : " -> ");
            }
            break;

        case DEADLOCK_REQUEST_WAITING:
            printf("Request is waiting for resources.\n");
            break;

        case DEADLOCK_REQUEST_EXCEEDS_NEED:
            printf("Request exceeds the process's remaining need.\n");
            break;

        case DEADLOCK_REQUEST_UNSAFE:
            printf("Request denied: it would make the system unsafe.\n");
            break;

        default:
            printf("Request rejected: invalid request or system state.\n");
            break;
    }
}

/*
 * Call this menu from the shared application when testing this
 * component independently. It does not create or own a main().
 */
void resource_management_menu(DeadlockSystem *system)
{
    int choice;

    if (system == NULL)
    {
        printf("Invalid system pointer.\n");
        return;
    }

    do
    {
        printf("\n===== RESOURCE MANAGEMENT =====\n");
        printf("1. Create Process\n");
        printf("2. Create Resource\n");
        printf("3. Set Allocation\n");
        printf("4. Set Maximum\n");
        printf("5. Display Matrices\n");
        printf("6. Run Banker's Algorithm\n");
        printf("7. Submit Resource Request\n");
        printf("8. Exit Resource Management Menu\n");

        if (!read_int("Enter choice: ", &choice))
            continue;

        switch (choice)
        {
            case 1: create_process(system); break;
            case 2: create_resource(system); break;
            case 3: set_allocation(system); break;
            case 4: set_maximum(system); break;
            case 5: display_matrices(system); break;
            case 6: run_bankers(system); break;
            case 7: submit_request(system); break;
            case 8: printf("Returning to the application.\n"); break;
            default: printf("Invalid choice.\n");
        }
    } while (choice != 8);
}
