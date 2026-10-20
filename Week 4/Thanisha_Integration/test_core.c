#include <stdio.h>
#include "deadlock.h"

int main(void)
{
    DeadlockSystem system;
    BankerResult result;

    deadlock_system_init(&system);

    if (!deadlock_add_process(&system, 1) ||
        !deadlock_add_process(&system, 2)) {
        printf("Failed to create processes.\n");
        return 1;
    }

    if (!deadlock_add_resource(&system, 3)) {
        printf("Failed to create resource.\n");
        return 1;
    }

    if (!deadlock_set_maximum(&system, 0, 0, 2) ||
        !deadlock_set_maximum(&system, 1, 0, 2)) {
        printf("Failed to set maximum requirements.\n");
        return 1;
    }

    if (!deadlock_set_allocation(&system, 0, 0, 1) ||
        !deadlock_set_allocation(&system, 1, 0, 1)) {
        printf("Failed to allocate resources.\n");
        return 1;
    }

    printf("Available resources: %d\n", system.available[0]);

    DeadlockSafetyStatus status =
        run_bankers_algorithm(&system, &result);

    if (status != DEADLOCK_SAFETY_SAFE) {
        printf("Banker's Algorithm did not return SAFE.\n");
        return 1;
    }

    printf("Banker's Algorithm: SAFE\n");
    printf("Safe sequence: ");

    for (int i = 0; i < result.safe_sequence_length; i++) {
        printf("P%d", result.safe_sequence_pids[i]);

        if (i + 1 < result.safe_sequence_length)
            printf(" -> ");
    }

    printf("\n");
    return 0;
}