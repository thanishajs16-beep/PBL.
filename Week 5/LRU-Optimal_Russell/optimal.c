#include "common.h"
#include <stdint.h>

PageReplacementStatus page_simulate_optimal(
    const PageReferenceInput *input,
    PageSimulationResult *result)
{
    PageSimulationResult output = {0};
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
        input->reference_count > SIZE_MAX / sizeof(*output.step_status)) {
        return PAGE_REPLACEMENT_SIZE_OVERFLOW;
    }

    output.algorithm = PAGE_ALGORITHM_OPTIMAL;
    output.reference_count = input->reference_count;
    output.frame_count = input->frame_count;
    output.frame_table = malloc(cell_count * sizeof(*output.frame_table));
    output.step_status = malloc(input->reference_count * sizeof(*output.step_status));

    if (output.frame_table == NULL || output.step_status == NULL) {
        page_simulation_result_destroy(&output);
        return PAGE_REPLACEMENT_OUT_OF_MEMORY;
    }

    for (i = 0; i < cell_count; ++i) {
        output.frame_table[i] = PAGE_FRAME_EMPTY;
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
        } else {
            size_t farthest_next_use = 0;

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
                for (j = 0; j < input->frame_count; ++j) {
                    int current_page = PAGE_FRAME_AT(&output, i - 1, j);
                    size_t next_use = input->reference_count;
                    size_t k;

                    for (k = i + 1; k < input->reference_count; ++k) {
                        if (input->pages[k] == current_page) {
                            next_use = k;
                            break;
                        }
                    }

                    if (next_use == input->reference_count) {
                        selected_frame = j;
                        break;
                    }

                    if (selected_frame == input->frame_count ||
                        next_use > farthest_next_use) {
                        selected_frame = j;
                        farthest_next_use = next_use;
                    }
                }
            }
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

    *result = output;
    return PAGE_REPLACEMENT_OK;
}
