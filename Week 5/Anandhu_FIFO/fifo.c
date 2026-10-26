#include "common.h"

#include <stdint.h> /* SIZE_MAX */
#include <string.h>

/*
 * Simulate FIFO (First-In, First-Out) page replacement.
 *
 * IMPORTANT:
 * - Pass a valid PageReferenceInput.
 * - Pass a zero-initialized PageSimulationResult, e.g.
 *       PageSimulationResult result = {0};
 * - On success, release result memory with page_simulation_result_destroy().
 */
PageReplacementStatus page_simulate_fifo(
    const PageReferenceInput *input,
    PageSimulationResult *result)
{
    PageSimulationResult output = {0};
    size_t table_cells;
    size_t filled_frames = 0;
    size_t next_to_replace = 0;
    size_t i;

    if (input == NULL || result == NULL) {
        return PAGE_REPLACEMENT_INVALID_ARGUMENT;
    }

    if (!page_reference_input_is_valid(input)) {
        return PAGE_REPLACEMENT_INVALID_INPUT;
    }

    /* Prevent overflow when calculating allocation sizes. */
    if (input->reference_count > SIZE_MAX / input->frame_count) {
        return PAGE_REPLACEMENT_SIZE_OVERFLOW;
    }

    table_cells = input->reference_count * input->frame_count;

    if (table_cells > SIZE_MAX / sizeof(*output.frame_table) ||
        input->reference_count > SIZE_MAX / sizeof(*output.step_status)) {
        return PAGE_REPLACEMENT_SIZE_OVERFLOW;
    }

    output.frame_table = malloc(table_cells * sizeof(*output.frame_table));
    output.step_status = malloc(
        input->reference_count * sizeof(*output.step_status));

    if (output.frame_table == NULL || output.step_status == NULL) {
        free(output.frame_table);
        free(output.step_status);
        return PAGE_REPLACEMENT_OUT_OF_MEMORY;
    }

    output.algorithm = PAGE_ALGORITHM_FIFO;
    output.reference_count = input->reference_count;
    output.frame_count = input->frame_count;

    /* Initially, all frames are empty. */
    for (i = 0; i < input->frame_count; ++i) {
        /* The first row is initialized below before it is copied. */
        (void)i;
    }

    {
        int *frames = malloc(input->frame_count * sizeof(*frames));

        if (frames == NULL) {
            free(output.frame_table);
            free(output.step_status);
            return PAGE_REPLACEMENT_OUT_OF_MEMORY;
        }

        for (i = 0; i < input->frame_count; ++i) {
            frames[i] = PAGE_FRAME_EMPTY;
        }

        for (i = 0; i < input->reference_count; ++i) {
            int requested_page = input->pages[i];
            size_t frame_index;
            int hit = 0;

            /* Check whether the requested page is already in memory. */
            for (frame_index = 0; frame_index < input->frame_count;
                 ++frame_index) {
                if (frames[frame_index] == requested_page) {
                    hit = 1;
                    break;
                }
            }

            if (hit) {
                ++output.page_hits;
                output.step_status[i] = PAGE_STEP_HIT;
            } else {
                ++output.page_faults;
                output.step_status[i] = PAGE_STEP_FAULT;

                if (filled_frames < input->frame_count) {
                    /* Fill unused frames in order before replacing pages. */
                    frames[filled_frames] = requested_page;
                    ++filled_frames;

                    if (filled_frames == input->frame_count) {
                        next_to_replace = 0;
                    }
                } else {
                    /* Replace the page that has been in memory the longest. */
                    frames[next_to_replace] = requested_page;
                    next_to_replace = (next_to_replace + 1) % input->frame_count;
                }
            }

            /* Save the complete frame state after this page request. */
            for (frame_index = 0; frame_index < input->frame_count;
                 ++frame_index) {
                PAGE_FRAME_AT(&output, i, frame_index) = frames[frame_index];
            }
        }

        free(frames);
    }

    output.hit_ratio_percent =
        ((double)output.page_hits / (double)output.reference_count) * 100.0;
    output.fault_ratio_percent =
        ((double)output.page_faults / (double)output.reference_count) * 100.0;

    /* Transfer ownership of the allocated arrays to the caller. */
    *result = output;
    return PAGE_REPLACEMENT_OK;
}


