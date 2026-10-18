#ifndef DEADLOCK_H
#define DEADLOCK_H

#define DEADLOCK_MAX_PROCESSES 20
#define DEADLOCK_MAX_RESOURCES 10
#define DEADLOCK_MAX_GRAPH_EDGES (DEADLOCK_MAX_PROCESSES * DEADLOCK_MAX_RESOURCES * 2)

typedef enum {
    DEADLOCK_SAFETY_NOT_CHECKED = 0,
    DEADLOCK_SAFETY_SAFE,
    DEADLOCK_SAFETY_UNSAFE,
    DEADLOCK_SAFETY_INVALID
} DeadlockSafetyStatus;

typedef enum {
    DEADLOCK_DETECTION_NOT_RUN = 0,
    DEADLOCK_DETECTION_NO_DEADLOCK,
    DEADLOCK_DETECTION_FOUND,
    DEADLOCK_DETECTION_INVALID
} DeadlockDetectionStatus;

typedef enum {
    DEADLOCK_REQUEST_GRANTED = 0,
    DEADLOCK_REQUEST_WAITING,
    DEADLOCK_REQUEST_EXCEEDS_NEED,
    DEADLOCK_REQUEST_UNSAFE,
    DEADLOCK_REQUEST_INVALID
} DeadlockRequestStatus;

typedef enum {
    DEADLOCK_RECOVERY_PREEMPT_RESOURCES = 0,
    DEADLOCK_RECOVERY_TERMINATE_PROCESS
} DeadlockRecoveryStrategy;

typedef enum {
    DEADLOCK_SYSTEM_STATUS_UNKNOWN = 0,
    DEADLOCK_SYSTEM_STATUS_SAFE,
    DEADLOCK_SYSTEM_STATUS_UNSAFE,
    DEADLOCK_SYSTEM_STATUS_DEADLOCKED,
    DEADLOCK_SYSTEM_STATUS_INVALID
} DeadlockSystemStatus;

typedef enum {
    RAG_NODE_PROCESS = 0,
    RAG_NODE_RESOURCE
} RAGNodeType;

typedef enum {
    RAG_EDGE_ALLOCATION = 0,
    RAG_EDGE_REQUEST
} RAGEdgeType;

typedef struct {
    int process_count;
    int resource_count;
    int pid[DEADLOCK_MAX_PROCESSES];
    int active[DEADLOCK_MAX_PROCESSES];
    int total[DEADLOCK_MAX_RESOURCES];
    int available[DEADLOCK_MAX_RESOURCES];
    int allocation[DEADLOCK_MAX_PROCESSES][DEADLOCK_MAX_RESOURCES];
    int maximum[DEADLOCK_MAX_PROCESSES][DEADLOCK_MAX_RESOURCES];
    int need[DEADLOCK_MAX_PROCESSES][DEADLOCK_MAX_RESOURCES];
    int request[DEADLOCK_MAX_PROCESSES][DEADLOCK_MAX_RESOURCES];
} DeadlockSystem;

typedef struct {
    int safe_sequence_pids[DEADLOCK_MAX_PROCESSES];
    int safe_sequence_length;
} BankerResult;

typedef struct {
    int deadlocked_pids[DEADLOCK_MAX_PROCESSES];
    int deadlocked_count;
} DeadlockDetectionResult;

typedef struct {
    RAGNodeType from_type;
    int from_index;
    RAGNodeType to_type;
    int to_index;
    RAGEdgeType edge_type;
    int instances;
} ResourceGraphEdge;

typedef struct {
    int edge_count;
    ResourceGraphEdge edges[DEADLOCK_MAX_GRAPH_EDGES];
} ResourceAllocationGraph;

void deadlock_system_init(DeadlockSystem *system);
int deadlock_add_process(DeadlockSystem *system, int pid);
int deadlock_add_resource(DeadlockSystem *system, int total_instances);
int deadlock_set_resource_total(DeadlockSystem *system, int resource_index, int total_instances);
int deadlock_set_process_active(DeadlockSystem *system, int process_index, int active);
int deadlock_set_allocation(DeadlockSystem *system, int process_index, int resource_index, int instances);
int deadlock_set_maximum(DeadlockSystem *system, int process_index, int resource_index, int instances);
int deadlock_set_request(DeadlockSystem *system, int process_index, int resource_index, int instances);
int deadlock_recalculate_available(DeadlockSystem *system);
int deadlock_recalculate_need(DeadlockSystem *system);
int deadlock_validate_system(const DeadlockSystem *system);

DeadlockSafetyStatus run_bankers_algorithm(const DeadlockSystem *system, BankerResult *result);
DeadlockRequestStatus request_resources(DeadlockSystem *system, int process_index, const int request_vector[DEADLOCK_MAX_RESOURCES], BankerResult *result);
DeadlockDetectionStatus detect_deadlock(const DeadlockSystem *system, DeadlockDetectionResult *result);
DeadlockSystemStatus evaluate_system_status(const DeadlockSystem *system, BankerResult *banker_result, DeadlockDetectionResult *detection_result);
int recover_from_deadlock(DeadlockSystem *system, int process_index, DeadlockRecoveryStrategy strategy);

int build_resource_allocation_graph(const DeadlockSystem *system, ResourceAllocationGraph *graph);
double resource_utilization_percent(const DeadlockSystem *system, int resource_index);
double overall_resource_utilization_percent(const DeadlockSystem *system);

void gui_run(DeadlockSystem *system);
int run_deadlock_application(void);

#endif
