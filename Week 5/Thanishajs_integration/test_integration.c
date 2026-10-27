
#include <stdio.h>
#include "common.h"

static void display_frame_table(
    const int *pages,
    const PageSimulationResult *result)
{
    printf("\n%s - Frame-by-Frame Simulation\n",
           page_algorithm_name(result->algorithm));

    printf("Page:  ");

    for (size_t i = 0; i < result->reference_count; i++) {
        printf("%4d", pages[i]);
    }

    printf("\nStatus:");

    for (size_t i = 0; i < result->reference_count; i++) {
        printf("%4s",
               result->step_status[i] == PAGE_STEP_HIT
                   ? "H"
                   : "F");
    }

    printf("\n");

    for (size_t frame = 0; frame < result->frame_count; frame++) {
        printf("Frame %zu:", frame + 1);

        for (size_t i = 0; i < result->reference_count; i++) {
            int page = PAGE_FRAME_AT(result, i, frame);

            if (page == PAGE_FRAME_EMPTY) {
                printf("%4s", "-");
            } else {
                printf("%4d", page);
            }
        }

        printf("\n");
    }

    printf("H = Hit, F = Fault\n");
}

int main(void)
{
    int pages[] = {7, 0, 1, 2, 0, 3, 0, 4, 2, 3, 0, 3, 2};

    PageReferenceInput input = {
        pages,
        sizeof(pages) / sizeof(pages[0]),
        3
    };

    PageComparisonResult comparison = {0};

    PageReplacementStatus status =
        page_simulate_all(&input, &comparison);

    if (status != PAGE_REPLACEMENT_OK) {
        printf("Comparison failed: %s\n",
               page_replacement_status_name(status));
        return 1;
    }

    printf("\nPage Replacement Algorithm Comparison\n");
    printf("-------------------------------------\n");

    for (int i = 0; i < PAGE_ALGORITHM_COUNT; i++) {
        PageSimulationResult *result = &comparison.results[i];

        display_frame_table(input.pages, result);

        printf("\n%-8s | Hits: %zu | Faults: %zu | "
               "Hit ratio: %.2f%% | Fault ratio: %.2f%%\n",
               page_algorithm_name(result->algorithm),
               result->page_hits,
               result->page_faults,
               result->hit_ratio_percent,
               result->fault_ratio_percent);
    }

    printf("\nMinimum page faults: %zu\n",
           comparison.minimum_page_faults);

    printf("Best algorithm(s): ");

    int first = 1;

    for (int i = 0; i < PAGE_ALGORITHM_COUNT; i++) {
        if (comparison.best_algorithms_mask & PAGE_ALGORITHM_BIT(i)) {
            if (!first) {
                printf(", ");
            }

            printf("%s", page_algorithm_name((PageAlgorithm)i));
            first = 0;
        }
    }

    printf("\n");

    page_comparison_result_destroy(&comparison);

    return 0;
}
