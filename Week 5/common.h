#ifndef COMMON_H
#define COMMON_H

#include <stddef.h>
#include <stdlib.h>

#define PAGE_FRAME_EMPTY (-1)
#define PAGE_FRAME_AT(result, reference_index, frame_index) \
    ((result)->frame_table[(reference_index) * (result)->frame_count + (frame_index)])
#define PAGE_ALGORITHM_BIT(algorithm) (1u << (unsigned int)(algorithm))

typedef enum {
    PAGE_ALGORITHM_FIFO = 0,
    PAGE_ALGORITHM_LRU,
    PAGE_ALGORITHM_OPTIMAL,
    PAGE_ALGORITHM_COUNT
} PageAlgorithm;

typedef enum {
    PAGE_CHOICE_FIFO = 1,
    PAGE_CHOICE_LRU,
    PAGE_CHOICE_OPTIMAL,
    PAGE_CHOICE_COMPARE_ALL
} PageAlgorithmChoice;

typedef enum {
    PAGE_STEP_FAULT = 0,
    PAGE_STEP_HIT = 1
} PageStepStatus;

typedef enum {
    PAGE_REPLACEMENT_OK = 0,
    PAGE_REPLACEMENT_INVALID_ARGUMENT,
    PAGE_REPLACEMENT_INVALID_INPUT,
    PAGE_REPLACEMENT_OUT_OF_MEMORY,
    PAGE_REPLACEMENT_SIZE_OVERFLOW
} PageReplacementStatus;

typedef struct {
    int *pages;
    size_t reference_count;
    size_t frame_count;
} PageReferenceInput;

typedef struct {
    PageAlgorithm algorithm;
    size_t reference_count;
    size_t frame_count;
    int *frame_table;
    PageStepStatus *step_status;
    size_t page_hits;
    size_t page_faults;
    double hit_ratio_percent;
    double fault_ratio_percent;
} PageSimulationResult;

typedef struct {
    PageSimulationResult results[PAGE_ALGORITHM_COUNT];
    size_t minimum_page_faults;
    unsigned int best_algorithms_mask;
} PageComparisonResult;

static inline int page_reference_input_is_valid(const PageReferenceInput *input)
{
    size_t i;

    if (input == NULL || input->pages == NULL ||
        input->reference_count == 0 || input->frame_count == 0) {
        return 0;
    }

    for (i = 0; i < input->reference_count; ++i) {
        if (input->pages[i] < 0) {
            return 0;
        }
    }

    return 1;
}

static inline const char *page_algorithm_name(PageAlgorithm algorithm)
{
    switch (algorithm) {
        case PAGE_ALGORITHM_FIFO:
            return "FIFO";
        case PAGE_ALGORITHM_LRU:
            return "LRU";
        case PAGE_ALGORITHM_OPTIMAL:
            return "Optimal";
        default:
            return "Unknown";
    }
}

static inline const char *page_replacement_status_name(PageReplacementStatus status)
{
    switch (status) {
        case PAGE_REPLACEMENT_OK:
            return "Success";
        case PAGE_REPLACEMENT_INVALID_ARGUMENT:
            return "Invalid argument";
        case PAGE_REPLACEMENT_INVALID_INPUT:
            return "Invalid input";
        case PAGE_REPLACEMENT_OUT_OF_MEMORY:
            return "Memory allocation failed";
        case PAGE_REPLACEMENT_SIZE_OVERFLOW:
            return "Input is too large to simulate";
        default:
            return "Unknown error";
    }
}

static inline void page_reference_input_destroy(PageReferenceInput *input)
{
    if (input == NULL) {
        return;
    }

    free(input->pages);
    input->pages = NULL;
    input->reference_count = 0;
    input->frame_count = 0;
}

static inline void page_simulation_result_destroy(PageSimulationResult *result)
{
    if (result == NULL) {
        return;
    }

    free(result->frame_table);
    free(result->step_status);
    *result = (PageSimulationResult){0};
}

static inline void page_comparison_result_destroy(PageComparisonResult *comparison)
{
    size_t i;

    if (comparison == NULL) {
        return;
    }

    for (i = 0; i < PAGE_ALGORITHM_COUNT; ++i) {
        page_simulation_result_destroy(&comparison->results[i]);
    }

    comparison->minimum_page_faults = 0;
    comparison->best_algorithms_mask = 0;
}

PageReplacementStatus page_simulate_fifo(
    const PageReferenceInput *input,
    PageSimulationResult *result);

PageReplacementStatus page_simulate_lru(
    const PageReferenceInput *input,
    PageSimulationResult *result);

PageReplacementStatus page_simulate_optimal(
    const PageReferenceInput *input,
    PageSimulationResult *result);

PageReplacementStatus page_simulate_all(
    const PageReferenceInput *input,
    PageComparisonResult *comparison);

#endif
