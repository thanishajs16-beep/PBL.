#include <stdio.h>
#include "deadlock.h"

int main(void)
{
    DeadlockSystem system;
    DeadlockDetectionResult result;
    ResourceAllocationGraph graph;

    deadlock_system_init(&system);

    if (!deadlock_add_process(&system, 1) ||
        !deadlock_add_process(&system, 2) ||
        !deadlock_add_resource(&system, 2)) {
        printf("Setup failed.\n");
        return 1;
    }

    /* Each process may need both resource instances. */
    if (!deadlock_set_maximum(&system, 0, 0, 2) ||
        !deadlock_set_maximum(&system, 1, 0, 2) ||
        !deadlock_set_allocation(&system, 0, 0, 1) ||
        !deadlock_set_allocation(&system, 1, 0, 1) ||
        !deadlock_set_request(&system, 0, 0, 1) ||
        !deadlock_set_request(&system, 1, 0, 1)) {
        printf("Resource setup failed.\n");
        return 1;
    }

    printf("Available resources: %d\n", system.available[0]);

    DeadlockDetectionStatus status =
        detect_deadlock(&system, &result);

    if (status != DEADLOCK_DETECTION_FOUND) {
        printf("Expected a deadlock, but it was not detected.\n");
        return 1;
    }

    printf("Deadlock detected. Processes:");
    for (int i = 0; i < result.deadlocked_count; i++)
        printf(" P%d", result.deadlocked_pids[i]);
    printf("\n");

    if (!build_resource_allocation_graph(&system, &graph)) {
        printf("RAG generation failed.\n");
        return 1;
    }

    printf("RAG generated. Edge count: %d\n", graph.edge_count);

    if (!recover_from_deadlock(
            &system, 0, DEADLOCK_RECOVERY_TERMINATE_PROCESS)) {
        printf("Recovery failed.\n");
        return 1;
    }

    printf("Recovery completed.\n");

    status = detect_deadlock(&system, &result);

    if (status != DEADLOCK_DETECTION_NO_DEADLOCK) {
        printf("Deadlock still exists after recovery.\n");
        return 1;
    }

    printf("Deadlock after recovery: NO_DEADLOCK\n");
    return 0;
}