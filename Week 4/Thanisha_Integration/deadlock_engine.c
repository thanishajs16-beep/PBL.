#include "deadlock.h"

#include <string.h>

static int in_range(int value, int limit)
{
    return value >= 0 && value < limit;
}

static int valid_system_pointer(const DeadlockSystem *system)
{
    return system != NULL;
}

void deadlock_system_init(DeadlockSystem *system)
{
    if (!valid_system_pointer(system))
        return;
    memset(system, 0, sizeof(*system));
}

int deadlock_recalculate_need(DeadlockSystem *system)
{
    int p, r;

    if (!valid_system_pointer(system))
        return 0;

    for (p = 0; p < system->process_count; p++) {
        for (r = 0; r < system->resource_count; r++) {
            system->need[p][r] = system->maximum[p][r] - system->allocation[p][r];
            if (system->need[p][r] < 0)
                return 0;
        }
    }
    return 1;
}

int deadlock_recalculate_available(DeadlockSystem *system)
{
    int p, r;

    if (!valid_system_pointer(system))
        return 0;

    for (r = 0; r < system->resource_count; r++) {
        int used = 0;
        for (p = 0; p < system->process_count; p++) {
            if (system->active[p])
                used += system->allocation[p][r];
        }
        system->available[r] = system->total[r] - used;
        if (system->available[r] < 0)
            return 0;
    }
    return 1;
}

int deadlock_validate_system(const DeadlockSystem *system)
{
    int p, r;

    if (!valid_system_pointer(system))
        return 0;
    if (system->process_count < 0 || system->process_count > DEADLOCK_MAX_PROCESSES)
        return 0;
    if (system->resource_count < 0 || system->resource_count > DEADLOCK_MAX_RESOURCES)
        return 0;

    for (r = 0; r < system->resource_count; r++) {
        int used = 0;
        if (system->total[r] < 0 || system->available[r] < 0)
            return 0;
        for (p = 0; p < system->process_count; p++) {
            if (system->allocation[p][r] < 0 || system->maximum[p][r] < 0 ||
                system->need[p][r] < 0 || system->request[p][r] < 0)
                return 0;
            if (system->allocation[p][r] > system->maximum[p][r])
                return 0;
            if (system->need[p][r] != system->maximum[p][r] - system->allocation[p][r])
                return 0;
            if (system->active[p])
                used += system->allocation[p][r];
        }
        if (used + system->available[r] != system->total[r])
            return 0;
    }
    return 1;
}

int deadlock_add_process(DeadlockSystem *system, int pid)
{
    int i, r, p;

    if (!valid_system_pointer(system) || system->process_count >= DEADLOCK_MAX_PROCESSES)
        return 0;

    for (i = 0; i < system->process_count; i++) {
        if (system->pid[i] == pid)
            return 0;
    }

    p = system->process_count;
    system->pid[p] = pid;
    system->active[p] = 1;
    for (r = 0; r < system->resource_count; r++) {
        system->allocation[p][r] = 0;
        system->maximum[p][r] = 0;
        system->need[p][r] = 0;
        system->request[p][r] = 0;
    }
    system->process_count++;
    return 1;
}

int deadlock_add_resource(DeadlockSystem *system, int total_instances)
{
    int p, r;

    if (!valid_system_pointer(system) || total_instances < 0 ||
        system->resource_count >= DEADLOCK_MAX_RESOURCES)
        return 0;

    r = system->resource_count;
    system->total[r] = total_instances;
    system->available[r] = total_instances;
    for (p = 0; p < system->process_count; p++) {
        system->allocation[p][r] = 0;
        system->maximum[p][r] = 0;
        system->need[p][r] = 0;
        system->request[p][r] = 0;
    }
    system->resource_count++;
    return deadlock_recalculate_available(system) && deadlock_recalculate_need(system);
}

int deadlock_set_resource_total(DeadlockSystem *system, int resource_index, int total_instances)
{
    int old;

    if (!valid_system_pointer(system) || total_instances < 0 ||
        !in_range(resource_index, system->resource_count))
        return 0;

    old = system->total[resource_index];
    system->total[resource_index] = total_instances;
    if (!deadlock_recalculate_available(system) || !deadlock_validate_system(system)) {
        system->total[resource_index] = old;
        deadlock_recalculate_available(system);
        return 0;
    }
    return 1;
}

int deadlock_set_process_active(DeadlockSystem *system, int process_index, int active)
{
    if (!valid_system_pointer(system) || !in_range(process_index, system->process_count))
        return 0;
    system->active[process_index] = active ? 1 : 0;
    return deadlock_recalculate_available(system);
}

int deadlock_set_allocation(DeadlockSystem *system, int process_index, int resource_index, int instances)
{
    int old;

    if (!valid_system_pointer(system) || instances < 0 ||
        !in_range(process_index, system->process_count) ||
        !in_range(resource_index, system->resource_count))
        return 0;
    if (instances > system->maximum[process_index][resource_index])
        return 0;

    old = system->allocation[process_index][resource_index];
    system->allocation[process_index][resource_index] = instances;
    if (!deadlock_recalculate_available(system) || !deadlock_recalculate_need(system) ||
        !deadlock_validate_system(system)) {
        system->allocation[process_index][resource_index] = old;
        deadlock_recalculate_available(system);
        deadlock_recalculate_need(system);
        return 0;
    }
    return 1;
}

int deadlock_set_maximum(DeadlockSystem *system, int process_index, int resource_index, int instances)
{
    int old;

    if (!valid_system_pointer(system) || instances < 0 ||
        !in_range(process_index, system->process_count) ||
        !in_range(resource_index, system->resource_count))
        return 0;
    if (instances < system->allocation[process_index][resource_index])
        return 0;
    if (instances > system->total[resource_index])
        return 0;

    old = system->maximum[process_index][resource_index];
    system->maximum[process_index][resource_index] = instances;
    if (!deadlock_recalculate_need(system) || !deadlock_validate_system(system)) {
        system->maximum[process_index][resource_index] = old;
        deadlock_recalculate_need(system);
        return 0;
    }
    return 1;
}

int deadlock_set_request(DeadlockSystem *system, int process_index, int resource_index, int instances)
{
    if (!valid_system_pointer(system) || instances < 0 ||
        !in_range(process_index, system->process_count) ||
        !in_range(resource_index, system->resource_count))
        return 0;
    system->request[process_index][resource_index] = instances;
    return 1;
}

/* Banker's safety algorithm from gtk_module.c, using Need <= Work. */
DeadlockSafetyStatus run_bankers_algorithm(const DeadlockSystem *system, BankerResult *result)
{
    int work[DEADLOCK_MAX_RESOURCES];
    int finish[DEADLOCK_MAX_PROCESSES];
    int count = 0;
    int active_needed = 0;
    int p, r;

    if (result) {
        result->safe_sequence_length = 0;
        memset(result->safe_sequence_pids, 0, sizeof(result->safe_sequence_pids));
    }
    if (!valid_system_pointer(system) || !system->process_count || !system->resource_count)
        return DEADLOCK_SAFETY_INVALID;
    if (!deadlock_validate_system(system))
        return DEADLOCK_SAFETY_INVALID;

    for (r = 0; r < system->resource_count; r++)
        work[r] = system->available[r];
    for (p = 0; p < system->process_count; p++) {
        finish[p] = !system->active[p];
        if (system->active[p])
            active_needed++;
    }

    while (count < active_needed) {
        int progressed = 0;
        for (p = 0; p < system->process_count; p++) {
            int can_finish = 1;
            if (finish[p])
                continue;
            for (r = 0; r < system->resource_count; r++) {
                if (system->need[p][r] > work[r]) {
                    can_finish = 0;
                    break;
                }
            }
            if (can_finish) {
                for (r = 0; r < system->resource_count; r++)
                    work[r] += system->allocation[p][r];
                finish[p] = 1;
                if (result && count < DEADLOCK_MAX_PROCESSES)
                    result->safe_sequence_pids[count] = system->pid[p];
                count++;
                progressed = 1;
            }
        }
        if (!progressed) {
            if (result)
                result->safe_sequence_length = 0;
            return DEADLOCK_SAFETY_UNSAFE;
        }
    }

    if (result)
        result->safe_sequence_length = count;
    return DEADLOCK_SAFETY_SAFE;
}

DeadlockRequestStatus request_resources(DeadlockSystem *system, int process_index,
                                        const int request_vector[DEADLOCK_MAX_RESOURCES],
                                        BankerResult *result)
{
    int r;
    int saved_available[DEADLOCK_MAX_RESOURCES];
    int saved_allocation[DEADLOCK_MAX_RESOURCES];
    int saved_need[DEADLOCK_MAX_RESOURCES];
    int saved_request[DEADLOCK_MAX_RESOURCES];

    if (result) {
        result->safe_sequence_length = 0;
        memset(result->safe_sequence_pids, 0, sizeof(result->safe_sequence_pids));
    }
    if (!valid_system_pointer(system) || request_vector == NULL ||
        !in_range(process_index, system->process_count) || !system->active[process_index] ||
        !deadlock_recalculate_need(system) || !deadlock_recalculate_available(system) ||
        !deadlock_validate_system(system))
        return DEADLOCK_REQUEST_INVALID;

    for (r = 0; r < system->resource_count; r++) {
        if (request_vector[r] < 0)
            return DEADLOCK_REQUEST_INVALID;
        if (request_vector[r] > system->need[process_index][r])
            return DEADLOCK_REQUEST_EXCEEDS_NEED;
    }
    for (r = 0; r < system->resource_count; r++) {
        if (request_vector[r] > system->available[r]) {
            for (r = 0; r < system->resource_count; r++)
                system->request[process_index][r] = request_vector[r];
            return DEADLOCK_REQUEST_WAITING;
        }
    }

    for (r = 0; r < system->resource_count; r++) {
        saved_available[r] = system->available[r];
        saved_allocation[r] = system->allocation[process_index][r];
        saved_need[r] = system->need[process_index][r];
        saved_request[r] = system->request[process_index][r];
        system->available[r] -= request_vector[r];
        system->allocation[process_index][r] += request_vector[r];
        system->need[process_index][r] -= request_vector[r];
        system->request[process_index][r] = 0;
    }

    if (run_bankers_algorithm(system, result) == DEADLOCK_SAFETY_SAFE)
        return DEADLOCK_REQUEST_GRANTED;

    for (r = 0; r < system->resource_count; r++) {
        system->available[r] = saved_available[r];
        system->allocation[process_index][r] = saved_allocation[r];
        system->need[process_index][r] = saved_need[r];
        system->request[process_index][r] = request_vector[r] ? request_vector[r] : saved_request[r];
    }
    if (result)
        result->safe_sequence_length = 0;
    return DEADLOCK_REQUEST_UNSAFE;
}

/* Detection algorithm from deadlock.c: Work/Finish with Request <= Work. */
DeadlockDetectionStatus detect_deadlock(const DeadlockSystem *system, DeadlockDetectionResult *result)
{
    int work[DEADLOCK_MAX_RESOURCES];
    int finish[DEADLOCK_MAX_PROCESSES];
    int p, r, changed;

    if (result) {
        result->deadlocked_count = 0;
        memset(result->deadlocked_pids, 0, sizeof(result->deadlocked_pids));
    }
    if (!valid_system_pointer(system) || !system->process_count || !system->resource_count)
        return DEADLOCK_DETECTION_INVALID;
    if (!deadlock_validate_system(system))
        return DEADLOCK_DETECTION_INVALID;

    for (r = 0; r < system->resource_count; r++)
        work[r] = system->available[r];

    for (p = 0; p < system->process_count; p++) {
        int has_allocation = 0;
        for (r = 0; r < system->resource_count; r++) {
            if (system->allocation[p][r] > 0)
                has_allocation = 1;
        }
        finish[p] = !system->active[p] || !has_allocation;
    }

    do {
        changed = 0;
        for (p = 0; p < system->process_count; p++) {
            int can_finish = 1;
            if (finish[p])
                continue;
            for (r = 0; r < system->resource_count; r++) {
                if (system->request[p][r] > work[r]) {
                    can_finish = 0;
                    break;
                }
            }
            if (can_finish) {
                for (r = 0; r < system->resource_count; r++)
                    work[r] += system->allocation[p][r];
                finish[p] = 1;
                changed = 1;
            }
        }
    } while (changed);

    for (p = 0; p < system->process_count; p++) {
        if (!finish[p] && system->active[p] && result &&
            result->deadlocked_count < DEADLOCK_MAX_PROCESSES) {
            result->deadlocked_pids[result->deadlocked_count++] = system->pid[p];
        }
    }

    if (result && result->deadlocked_count > 0)
        return DEADLOCK_DETECTION_FOUND;
    for (p = 0; p < system->process_count; p++) {
        if (!finish[p] && system->active[p])
            return DEADLOCK_DETECTION_FOUND;
    }
    return DEADLOCK_DETECTION_NO_DEADLOCK;
}

DeadlockSystemStatus evaluate_system_status(const DeadlockSystem *system, BankerResult *banker_result,
                                            DeadlockDetectionResult *detection_result)
{
    DeadlockDetectionStatus detection = detect_deadlock(system, detection_result);
    DeadlockSafetyStatus safety = run_bankers_algorithm(system, banker_result);

    if (detection == DEADLOCK_DETECTION_INVALID || safety == DEADLOCK_SAFETY_INVALID)
        return DEADLOCK_SYSTEM_STATUS_INVALID;
    if (detection == DEADLOCK_DETECTION_FOUND)
        return DEADLOCK_SYSTEM_STATUS_DEADLOCKED;
    if (safety == DEADLOCK_SAFETY_SAFE)
        return DEADLOCK_SYSTEM_STATUS_SAFE;
    return DEADLOCK_SYSTEM_STATUS_UNSAFE;
}

int recover_from_deadlock(DeadlockSystem *system, int process_index, DeadlockRecoveryStrategy strategy)
{
    int r;

    if (!valid_system_pointer(system) || !in_range(process_index, system->process_count))
        return 0;
    if (!system->active[process_index])
        return 0;

    for (r = 0; r < system->resource_count; r++) {
        system->available[r] += system->allocation[process_index][r];
        system->allocation[process_index][r] = 0;
        system->request[process_index][r] = 0;
        if (strategy == DEADLOCK_RECOVERY_TERMINATE_PROCESS)
            system->need[process_index][r] = 0;
        else
            system->need[process_index][r] = system->maximum[process_index][r];
    }
    if (strategy == DEADLOCK_RECOVERY_TERMINATE_PROCESS)
        system->active[process_index] = 0;

    return deadlock_recalculate_available(system) && deadlock_recalculate_need(system);
}

int build_resource_allocation_graph(const DeadlockSystem *system, ResourceAllocationGraph *graph)
{
    int p, r;

    if (!graph)
        return 0;
    graph->edge_count = 0;
    if (!valid_system_pointer(system))
        return 0;

    for (p = 0; p < system->process_count; p++) {
        if (!system->active[p])
            continue;
        for (r = 0; r < system->resource_count; r++) {
            if (system->allocation[p][r] > 0 && graph->edge_count < DEADLOCK_MAX_GRAPH_EDGES) {
                ResourceGraphEdge *edge = &graph->edges[graph->edge_count++];
                edge->from_type = RAG_NODE_RESOURCE;
                edge->from_index = r;
                edge->to_type = RAG_NODE_PROCESS;
                edge->to_index = p;
                edge->edge_type = RAG_EDGE_ALLOCATION;
                edge->instances = system->allocation[p][r];
            }
            if (system->request[p][r] > 0 && graph->edge_count < DEADLOCK_MAX_GRAPH_EDGES) {
                ResourceGraphEdge *edge = &graph->edges[graph->edge_count++];
                edge->from_type = RAG_NODE_PROCESS;
                edge->from_index = p;
                edge->to_type = RAG_NODE_RESOURCE;
                edge->to_index = r;
                edge->edge_type = RAG_EDGE_REQUEST;
                edge->instances = system->request[p][r];
            }
        }
    }
    return 1;
}

double resource_utilization_percent(const DeadlockSystem *system, int resource_index)
{
    int used;

    if (!valid_system_pointer(system) || !in_range(resource_index, system->resource_count) ||
        system->total[resource_index] <= 0)
        return 0.0;
    used = system->total[resource_index] - system->available[resource_index];
    return 100.0 * (double)used / (double)system->total[resource_index];
}

double overall_resource_utilization_percent(const DeadlockSystem *system)
{
    int r, total = 0, used = 0;

    if (!valid_system_pointer(system) || system->resource_count == 0)
        return 0.0;
    for (r = 0; r < system->resource_count; r++) {
        total += system->total[r];
        used += system->total[r] - system->available[r];
    }
    if (total <= 0)
        return 0.0;
    return 100.0 * (double)used / (double)total;
}
