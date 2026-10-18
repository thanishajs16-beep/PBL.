#include "../deadlock.h"
#include <stdio.h>
void display_system_matrices(const DeadlockSystem *system);
void display_system_status(const DeadlockSystem *system);

/* Create processes and resources, and update resource data. */

void resource_management_menu(DeadlockSystem *system)
{
    int choice;
    int process_index, resource_index, value;
    int pid;

    do {
        printf("\n===== RESOURCE MANAGEMENT =====\n");
        printf("1. Create Process\n");
        printf("2. Create Resource\n");
        printf("3. Set Allocation\n");
        printf("4. Set Maximum\n");
        printf("5. Set Request\n");
        printf("6. Run Banker's Algorithm\n");
        printf("7. Display System Matrices\n");
        printf("8. Display System Status\n");
        printf("0. Return to Main Menu\n");
        printf("Enter choice: ");

        if (scanf("%d", &choice) != 1) {
            printf("Invalid input.\n");
            return;
        }

        switch (choice) {
        case 1:
            printf("Enter process ID: ");
            if (scanf("%d", &pid) != 1) {
                printf("Invalid input.\n");
                return;
            }

            if (deadlock_add_process(system, pid))
                printf("Process created successfully.\n");
            else
                printf("Could not create process.\n");
            break;

        case 2:
            printf("Enter total resource instances: ");
            if (scanf("%d", &value) != 1) {
                printf("Invalid input.\n");
                return;
            }

            if (deadlock_add_resource(system, value))
                printf("Resource created successfully.\n");
            else
                printf("Could not create resource.\n");
            break;

        case 3:
        case 4:
        case 5:
            printf("Enter process index (0 to %d): ",
                   system->process_count - 1);
            if (scanf("%d", &process_index) != 1) {
                printf("Invalid input.\n");
                return;
            }

            printf("Enter resource index (0 to %d): ",
                   system->resource_count - 1);
            if (scanf("%d", &resource_index) != 1) {
                printf("Invalid input.\n");
                return;
            }

            printf("Enter number of instances: ");
            if (scanf("%d", &value) != 1) {
                printf("Invalid input.\n");
                return;
            }

            if (choice == 3)
                value = deadlock_set_allocation(
                    system, process_index, resource_index, value);
            else if (choice == 4)
                value = deadlock_set_maximum(
                    system, process_index, resource_index, value);
            else
                value = deadlock_set_request(
                    system, process_index, resource_index, value);

            if (value)
                printf("Value updated successfully.\n");
            else
                printf("Update failed. Check the indices and value.\n");
            break;

        case 6: {
            BankerResult result;
            DeadlockSafetyStatus status =
                run_bankers_algorithm(system, &result);

            if (status == DEADLOCK_SAFETY_SAFE) {
                printf("System Status: SAFE\n");
                printf("Safe Sequence: ");

                for (int i = 0; i < result.safe_sequence_length; i++) {
                    printf("P%d",
                           result.safe_sequence_pids[i]);
                    if (i + 1 < result.safe_sequence_length)
                        printf(" -> ");
                }
                printf("\n");
            } else if (status == DEADLOCK_SAFETY_UNSAFE) {
                printf("System Status: UNSAFE\n");
            } else {
                printf("Unable to determine system safety.\n");
            }
            break;
        }

        case 7:
            display_system_matrices(system);
            break;

        case 8:
            display_system_status(system);
            break;

        case 0:
            printf("Returning to main menu.\n");
            break;

        default:
            printf("Invalid choice. Try again.\n");
        }

    } while (choice != 0);
}
void display_system_matrices(const DeadlockSystem *system)
{
    int i, j;

    printf("\n===== SYSTEM MATRICES =====\n");

    printf("\nAvailable:\n");
    for (j = 0; j < system->resource_count; j++)
        printf("R%d ", j);
    printf("\n");
    for (j = 0; j < system->resource_count; j++)
        printf("%d  ", system->available[j]);
    printf("\n");

    printf("\nAllocation Matrix:\n");
    printf("PID\t");
    for (j = 0; j < system->resource_count; j++)
        printf("R%d\t", j);
    printf("\n");

    for (i = 0; i < system->process_count; i++) {
        printf("P%d\t", system->pid[i]);
        for (j = 0; j < system->resource_count; j++)
            printf("%d\t", system->allocation[i][j]);
        printf("\n");
    }

    printf("\nMaximum Matrix:\n");
    printf("PID\t");
    for (j = 0; j < system->resource_count; j++)
        printf("R%d\t", j);
    printf("\n");

    for (i = 0; i < system->process_count; i++) {
        printf("P%d\t", system->pid[i]);
        for (j = 0; j < system->resource_count; j++)
            printf("%d\t", system->maximum[i][j]);
        printf("\n");
    }

    printf("\nNeed Matrix:\n");
    printf("PID\t");
    for (j = 0; j < system->resource_count; j++)
        printf("R%d\t", j);
    printf("\n");

    for (i = 0; i < system->process_count; i++) {
        printf("P%d\t", system->pid[i]);
        for (j = 0; j < system->resource_count; j++)
            printf("%d\t", system->need[i][j]);
        printf("\n");
    }
}

void display_system_status(const DeadlockSystem *system)
{
    BankerResult banker_result;
    DeadlockDetectionResult detection_result;

    DeadlockSystemStatus status = evaluate_system_status(
        system, &banker_result, &detection_result);

    printf("\n===== SYSTEM STATUS =====\n");

    switch (status) {
    case DEADLOCK_SYSTEM_STATUS_SAFE:
        printf("System Status: SAFE\n");
        printf("Safe Sequence: ");
        for (int i = 0; i < banker_result.safe_sequence_length; i++) {
            printf("P%d", banker_result.safe_sequence_pids[i]);
            if (i + 1 < banker_result.safe_sequence_length)
                printf(" -> ");
        }
        printf("\n");
        break;

    case DEADLOCK_SYSTEM_STATUS_UNSAFE:
        printf("System Status: UNSAFE\n");
        break;

    case DEADLOCK_SYSTEM_STATUS_DEADLOCKED:
        printf("System Status: DEADLOCKED\n");
        printf("Deadlocked processes: ");
        for (int i = 0; i < detection_result.deadlocked_count; i++) {
            printf("P%d", detection_result.deadlocked_pids[i]);
            if (i + 1 < detection_result.deadlocked_count)
                printf(", ");
        }
        printf("\n");
        break;

    default:
        printf("System status is invalid or unknown.\n");
    }
}