
#include "common.h"

PageReplacementStatus page_simulate_all(
    const PageReferenceInput *input,
    PageComparisonResult *comparison)
{
    if (input == NULL || comparison == NULL) {
        return PAGE_REPLACEMENT_INVALID_ARGUMENT;
    }

    if (!page_reference_input_is_valid(input)) {
        return PAGE_REPLACEMENT_INVALID_INPUT;
    }

    *comparison = (PageComparisonResult){0};

    PageReplacementStatus status;

    status = page_simulate_fifo(
        input, &comparison->results[PAGE_ALGORITHM_FIFO]);

    if (status != PAGE_REPLACEMENT_OK) {
        page_comparison_result_destroy(comparison);
        return status;
    }

    status = page_simulate_lru(
        input, &comparison->results[PAGE_ALGORITHM_LRU]);

    if (status != PAGE_REPLACEMENT_OK) {
        page_comparison_result_destroy(comparison);
        return status;
    }

    status = page_simulate_optimal(
        input, &comparison->results[PAGE_ALGORITHM_OPTIMAL]);

    if (status != PAGE_REPLACEMENT_OK) {
        page_comparison_result_destroy(comparison);
        return status;
    }

    comparison->minimum_page_faults =
        comparison->results[0].page_faults;

    for (int i = 1; i < PAGE_ALGORITHM_COUNT; i++) {
        if (comparison->results[i].page_faults <
            comparison->minimum_page_faults) {
            comparison->minimum_page_faults =
                comparison->results[i].page_faults;
        }
    }

    comparison->best_algorithms_mask = 0;

    for (int i = 0; i < PAGE_ALGORITHM_COUNT; i++) {
        if (comparison->results[i].page_faults ==
            comparison->minimum_page_faults) {
            comparison->best_algorithms_mask |=
                PAGE_ALGORITHM_BIT(i);
        }
    }

    return PAGE_REPLACEMENT_OK;
}

