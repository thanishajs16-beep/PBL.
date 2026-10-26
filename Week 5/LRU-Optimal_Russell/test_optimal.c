#include "common.h"
#include <stdio.h>

int main(void)
{
    int pages[] = {7, 0, 1, 2, 0, 3, 0, 4, 2, 3};
    PageReferenceInput input = {
        pages,
        sizeof(pages) / sizeof(pages[0]),
        3
    };
    PageSimulationResult result = {0};
    PageReplacementStatus status;
    size_t i;
    size_t j;

    status = page_simulate_optimal(&input, &result);
    if (status != PAGE_REPLACEMENT_OK) {
        fprintf(stderr, "Optimal simulation failed: %s\n",
                page_replacement_status_name(status));
        return 1;
    }

    printf("Page\tFrame 1\tFrame 2\tFrame 3\tStatus\n");
    for (i = 0; i < result.reference_count; ++i) {
        printf("%d\t", pages[i]);
        for (j = 0; j < result.frame_count; ++j) {
            int frame = PAGE_FRAME_AT(&result, i, j);
            if (frame == PAGE_FRAME_EMPTY) {
                printf("-\t");
            } else {
                printf("%d\t", frame);
            }
        }
        printf("%s\n", result.step_status[i] == PAGE_STEP_HIT ? "Hit" : "Fault");
    }

    printf("Page references: %zu\n", result.reference_count);
    printf("Page hits: %zu\n", result.page_hits);
    printf("Page faults: %zu\n", result.page_faults);
    printf("Hit ratio: %.2f%%\n", result.hit_ratio_percent);
    printf("Fault ratio: %.2f%%\n", result.fault_ratio_percent);

    page_simulation_result_destroy(&result);
    return 0;
}
