#include "common.h"
#include <stdint.h>

PageReplacementStatus page_simulate_lru(
    const PageReferenceInput *input,
    PageSimulationResult *result)
{
    PageSimulationResult output = {0};
    size_t *last_used = NULL;
    size_t cell_count;
    size_t i;
    size_t j;

    if (result == NULL) {
        return PAGE_REPLACEMENT_INVALID_ARGUMENT;
    }

    *result = (PageSimulationResult){0};

    if (!page_reference_input_is_valid(input)) {
        return PAGE_REPLACEMENT_INVALID_INPUT;
    }

    if (input->reference_count > SIZE_MAX / input->frame_count) {
        return PAGE_REPLACEMENT_SIZE_OVERFLOW;
    }

    cell_count = input->reference_count * input->frame_count;

    if (cell_count > SIZE_MAX / sizeof(*output.frame_table) ||
        input->reference_count > SIZE_MAX / sizeof(*output.step_status) ||
        input->frame_count > SIZE_MAX / sizeof(*last_used)) {
        return PAGE_REPLACEMENT_SIZE_OVERFLOW;
    }

    output.algorithm = PAGE_ALGORITHM_LRU;
    output.reference_count = input->reference_count;
    output.frame_count = input->frame_count;
    output.frame_table = malloc(cell_count * sizeof(*output.frame_table));
    output.step_status = malloc(input->reference_count * sizeof(*output.step_status));
    last_used = malloc(input->frame_count * sizeof(*last_used));

    if (output.frame_table == NULL || output.step_status == NULL || last_used == NULL) {
        free(last_used);
        page_simulation_result_destroy(&output);
        return PAGE_REPLACEMENT_OUT_OF_MEMORY;
    }

    for (i = 0; i < cell_count; ++i) {
        output.frame_table[i] = PAGE_FRAME_EMPTY;
    }

    for (i = 0; i < input->frame_count; ++i) {
        last_used[i] = 0;
    }

    for (i = 0; i < input->reference_count; ++i) {
        int page = input->pages[i];
        size_t selected_frame = input->frame_count;

        for (j = 0; j < input->frame_count; ++j) {
            int current_page = (i == 0)
                ? PAGE_FRAME_EMPTY
                : PAGE_FRAME_AT(&output, i - 1, j);

            if (current_page == page) {
                selected_frame = j;
                break;
            }
        }

        if (selected_frame != input->frame_count) {
            output.step_status[i] = PAGE_STEP_HIT;
            output.page_hits++;
            last_used[selected_frame] = i;
        } else {
            output.step_status[i] = PAGE_STEP_FAULT;
            output.page_faults++;

            for (j = 0; j < input->frame_count; ++j) {
                int current_page = (i == 0)
                    ? PAGE_FRAME_EMPTY
                    : PAGE_FRAME_AT(&output, i - 1, j);

                if (current_page == PAGE_FRAME_EMPTY) {
                    selected_frame = j;
                    break;
                }
            }

            if (selected_frame == input->frame_count) {
                selected_frame = 0;
                for (j = 1; j < input->frame_count; ++j) {
                    if (last_used[j] < last_used[selected_frame]) {
                        selected_frame = j;
                    }
                }
            }

            last_used[selected_frame] = i;
        }

        for (j = 0; j < input->frame_count; ++j) {
            int previous_page = (i == 0)
                ? PAGE_FRAME_EMPTY
                : PAGE_FRAME_AT(&output, i - 1, j);
            PAGE_FRAME_AT(&output, i, j) = previous_page;
        }

        PAGE_FRAME_AT(&output, i, selected_frame) = page;
    }

    output.hit_ratio_percent =
        ((double)output.page_hits / (double)output.reference_count) * 100.0;
    output.fault_ratio_percent =
        ((double)output.page_faults / (double)output.reference_count) * 100.0;

    free(last_used);
    *result = output;
    return PAGE_REPLACEMENT_OK;
}
