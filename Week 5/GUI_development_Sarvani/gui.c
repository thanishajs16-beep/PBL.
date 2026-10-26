#include <gtk/gtk.h>
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "common.h"

typedef struct {
    GtkWidget *window;
    GtkWidget *reference_entry;
    GtkWidget *frames_entry;
    GtkWidget *algorithm_combo;
    GtkWidget *notebook;
    GtkWidget *status_label;
} AppWidgets;

static void show_error(GtkWindow *parent, const char *message)
{
    GtkWidget *dialog = gtk_message_dialog_new(
        parent,
        GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
        GTK_MESSAGE_ERROR,
        GTK_BUTTONS_OK,
        "%s", message);
    gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);
}

/* Accepts page numbers separated by spaces, tabs, or commas. */
static int parse_reference_string(const char *text, PageReferenceInput *input,
                                  char *error_message, size_t error_size)
{
    const char *p = text;
    size_t count = 0;
    size_t capacity = 16;
    int *pages = NULL;

    if (text == NULL || input == NULL) {
        snprintf(error_message, error_size, "Missing page-reference input.");
        return 0;
    }

    pages = malloc(capacity * sizeof(*pages));
    if (pages == NULL) {
        snprintf(error_message, error_size, "Not enough memory to read the input.");
        return 0;
    }

    while (*p != '\0') {
        char *end = NULL;
        long value;

        while (g_ascii_isspace(*p) || *p == ',') {
            ++p;
        }
        if (*p == '\0') {
            break;
        }
        if (!g_ascii_isdigit(*p)) {
            snprintf(error_message, error_size,
                     "Invalid page-reference string. Use non-negative integers separated by spaces or commas.");
            free(pages);
            return 0;
        }

        errno = 0;
        value = strtol(p, &end, 10);
        if (errno == ERANGE || end == p || value < 0 || value > INT_MAX) {
            snprintf(error_message, error_size,
                     "Each page number must be between 0 and %d.", INT_MAX);
            free(pages);
            return 0;
        }
        if (*end != '\0' && !g_ascii_isspace(*end) && *end != ',') {
            snprintf(error_message, error_size,
                     "Invalid character in page-reference string. Use spaces or commas between numbers.");
            free(pages);
            return 0;
        }

        if (count >= 10000) {
            snprintf(error_message, error_size,
                     "Please enter no more than 10000 page references.");
            free(pages);
            return 0;
        }
        if (count == capacity) {
            size_t new_capacity = capacity * 2;
            int *new_pages = realloc(pages, new_capacity * sizeof(*pages));
            if (new_pages == NULL) {
                snprintf(error_message, error_size, "Not enough memory to read the input.");
                free(pages);
                return 0;
            }
            pages = new_pages;
            capacity = new_capacity;
        }

        pages[count++] = (int)value;
        p = end;
    }

    if (count == 0) {
        snprintf(error_message, error_size, "Enter at least one page number.");
        free(pages);
        return 0;
    }

    input->pages = pages;
    input->reference_count = count;
    input->frame_count = 0;
    return 1;
}

static int parse_frame_count(const char *text, size_t *frame_count,
                             char *error_message, size_t error_size)
{
    char *end = NULL;
    long value;

    if (text == NULL || *text == '\0') {
        snprintf(error_message, error_size, "Enter the number of frames.");
        return 0;
    }

    errno = 0;
    value = strtol(text, &end, 10);
    while (end != NULL && g_ascii_isspace(*end)) {
        ++end;
    }
    if (errno == ERANGE || end == text || *end != '\0' || value <= 0 || value > 100) {
        snprintf(error_message, error_size,
                 "Frame count must be a positive whole number from 1 to 100.");
        return 0;
    }

    *frame_count = (size_t)value;
    return 1;
}

static void clear_results(AppWidgets *app)
{
    while (gtk_notebook_get_n_pages(GTK_NOTEBOOK(app->notebook)) > 0) {
        gtk_notebook_remove_page(GTK_NOTEBOOK(app->notebook), 0);
    }
}

static void set_store_text(GtkListStore *store, GtkTreeIter *iter,
                           int column, const char *text)
{
    GValue value = G_VALUE_INIT;
    g_value_init(&value, G_TYPE_STRING);
    g_value_set_string(&value, text);
    gtk_list_store_set_value(store, iter, column, &value);
    g_value_unset(&value);
}

static GtkWidget *create_result_page(const PageSimulationResult *result,
                                     const PageReferenceInput *input)
{
    GtkWidget *outer = gtk_box_new(GTK_ORIENTATION_VERTICAL, 8);
    GtkWidget *summary;
    GtkWidget *scrolled;
    GtkWidget *tree;
    GtkListStore *store;
    GtkTreeViewColumn *column;
    GtkCellRenderer *renderer;
    GtkTreeIter iter;
    size_t i, frame;
    int column_count = (int)result->frame_count + 2;
    char *summary_text = g_strdup_printf(
        "Algorithm: %s     References: %zu     Hits: %zu     Faults: %zu\n"
        "Hit ratio: %.2f%%     Fault ratio: %.2f%%",
        page_algorithm_name(result->algorithm), result->reference_count,
        result->page_hits, result->page_faults,
        result->hit_ratio_percent, result->fault_ratio_percent);

    summary = gtk_label_new(summary_text);
    g_free(summary_text);
    gtk_label_set_xalign(GTK_LABEL(summary), 0.0f);
    gtk_label_set_line_wrap(GTK_LABEL(summary), TRUE);
    gtk_box_pack_start(GTK_BOX(outer), summary, FALSE, FALSE, 6);

    GType *types = g_new(GType, column_count);
    for (int c = 0; c < column_count; ++c) {
        types[c] = G_TYPE_STRING;
    }
    store = gtk_list_store_newv(column_count, types);
    g_free(types);

    tree = gtk_tree_view_new_with_model(GTK_TREE_MODEL(store));
    g_object_unref(store);
    renderer = gtk_cell_renderer_text_new();

    column = gtk_tree_view_column_new_with_attributes("Page", renderer, "text", 0, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(tree), column);

    for (frame = 0; frame < result->frame_count; ++frame) {
        char heading[40];
        snprintf(heading, sizeof(heading), "Frame %zu", frame + 1);
        renderer = gtk_cell_renderer_text_new();
        column = gtk_tree_view_column_new_with_attributes(heading, renderer,
                                                          "text", (int)frame + 1, NULL);
        gtk_tree_view_append_column(GTK_TREE_VIEW(tree), column);
    }
    renderer = gtk_cell_renderer_text_new();
    column = gtk_tree_view_column_new_with_attributes("Status", renderer,
                                                       "text", column_count - 1, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(tree), column);

    for (i = 0; i < result->reference_count; ++i) {
        char buffer[32];
        gtk_list_store_append(store, &iter);
        snprintf(buffer, sizeof(buffer), "%d", input->pages[i]);
        set_store_text(store, &iter, 0, buffer);
        for (frame = 0; frame < result->frame_count; ++frame) {
            int page = PAGE_FRAME_AT(result, i, frame);
            if (page == PAGE_FRAME_EMPTY) {
                set_store_text(store, &iter, (int)frame + 1, "—");
            } else {
                snprintf(buffer, sizeof(buffer), "%d", page);
                set_store_text(store, &iter, (int)frame + 1, buffer);
            }
        }
        set_store_text(store, &iter, column_count - 1,
                       result->step_status[i] == PAGE_STEP_HIT ? "Hit" : "Fault");
    }

    scrolled = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled),
                                   GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_container_add(GTK_CONTAINER(scrolled), tree);
    gtk_box_pack_start(GTK_BOX(outer), scrolled, TRUE, TRUE, 0);
    g_object_set_data(G_OBJECT(outer), "result-store", store);
    return outer;
}

static void add_result_page(AppWidgets *app, const PageSimulationResult *result,
                            const PageReferenceInput *input)
{
    GtkWidget *page = create_result_page(result, input);
    gtk_notebook_append_page(GTK_NOTEBOOK(app->notebook), page,
                             gtk_label_new(page_algorithm_name(result->algorithm)));
    gtk_widget_show_all(page);
}

static void add_comparison_summary(AppWidgets *app, const PageComparisonResult *comparison)
{
    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    GtkWidget *label;
    GtkWidget *scrolled;
    GtkWidget *tree;
    GtkListStore *store;
    GtkTreeIter iter;
    GtkCellRenderer *renderer;
    const char *best = "";
    char *summary;

    if (comparison->best_algorithms_mask & PAGE_ALGORITHM_BIT(PAGE_ALGORITHM_FIFO)) best = "FIFO";
    if (comparison->best_algorithms_mask & PAGE_ALGORITHM_BIT(PAGE_ALGORITHM_LRU))
        best = (*best == '\0') ? "LRU" : "FIFO and LRU";
    if (comparison->best_algorithms_mask & PAGE_ALGORITHM_BIT(PAGE_ALGORITHM_OPTIMAL)) {
        if (*best == '\0') best = "Optimal";
        else if (strcmp(best, "FIFO and LRU") == 0) best = "FIFO, LRU and Optimal";
        else if (strcmp(best, "FIFO") == 0) best = "FIFO and Optimal";
        else best = "LRU and Optimal";
    }

    summary = g_strdup_printf("Minimum page faults: %zu\nBest result(s): %s",
                              comparison->minimum_page_faults,
                              *best ? best : "Not available");
    label = gtk_label_new(summary);
    g_free(summary);
    gtk_label_set_xalign(GTK_LABEL(label), 0.0f);
    gtk_box_pack_start(GTK_BOX(box), label, FALSE, FALSE, 8);

    store = gtk_list_store_new(4, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING);
    tree = gtk_tree_view_new_with_model(GTK_TREE_MODEL(store));
    g_object_unref(store);
    const char *headings[] = {"Algorithm", "Page Hits", "Page Faults", "Fault Ratio"};
    for (int c = 0; c < 4; ++c) {
        renderer = gtk_cell_renderer_text_new();
        GtkTreeViewColumn *col = gtk_tree_view_column_new_with_attributes(
            headings[c], renderer, "text", c, NULL);
        gtk_tree_view_append_column(GTK_TREE_VIEW(tree), col);
    }
    for (int a = 0; a < PAGE_ALGORITHM_COUNT; ++a) {
        const PageSimulationResult *r = &comparison->results[a];
        char hits[32], faults[32], ratio[32];
        snprintf(hits, sizeof(hits), "%zu", r->page_hits);
        snprintf(faults, sizeof(faults), "%zu", r->page_faults);
        snprintf(ratio, sizeof(ratio), "%.2f%%", r->fault_ratio_percent);
        gtk_list_store_append(store, &iter);
        gtk_list_store_set(store, &iter,
                           0, page_algorithm_name((PageAlgorithm)a),
                           1, hits, 2, faults, 3, ratio, -1);
    }
    scrolled = gtk_scrolled_window_new(NULL, NULL);
    gtk_container_add(GTK_CONTAINER(scrolled), tree);
    gtk_box_pack_start(GTK_BOX(box), scrolled, TRUE, TRUE, 0);
    gtk_notebook_append_page(GTK_NOTEBOOK(app->notebook), box,
                             gtk_label_new("Comparison"));
    gtk_widget_show_all(box);
}

static void on_run_clicked(GtkButton *button, gpointer user_data)
{
    AppWidgets *app = user_data;
    PageReferenceInput input = {0};
    char error_message[256];
    const char *reference_text = gtk_entry_get_text(GTK_ENTRY(app->reference_entry));
    const char *frames_text = gtk_entry_get_text(GTK_ENTRY(app->frames_entry));
    gint choice = gtk_combo_box_get_active(GTK_COMBO_BOX(app->algorithm_combo));
    PageReplacementStatus status;
    (void)button;

    if (!parse_reference_string(reference_text, &input, error_message, sizeof(error_message))) {
        show_error(GTK_WINDOW(app->window), error_message);
        return;
    }
    if (!parse_frame_count(frames_text, &input.frame_count, error_message, sizeof(error_message))) {
        page_reference_input_destroy(&input);
        show_error(GTK_WINDOW(app->window), error_message);
        return;
    }
    if (!page_reference_input_is_valid(&input)) {
        page_reference_input_destroy(&input);
        show_error(GTK_WINDOW(app->window), "The inputs are invalid.");
        return;
    }
    if (choice < 0 || choice > 3) {
        page_reference_input_destroy(&input);
        show_error(GTK_WINDOW(app->window), "Select an algorithm first.");
        return;
    }

    clear_results(app);
    gtk_label_set_text(GTK_LABEL(app->status_label), "Running simulation...");

    if (choice == 3) {
        PageComparisonResult comparison = {0};
        status = page_simulate_all(&input, &comparison);
        if (status != PAGE_REPLACEMENT_OK) {
            char *message = g_strdup_printf("Simulation failed: %s",
                                            page_replacement_status_name(status));
            show_error(GTK_WINDOW(app->window), message);
            gtk_label_set_text(GTK_LABEL(app->status_label), "Simulation failed.");
            g_free(message);
        } else {
            for (int a = 0; a < PAGE_ALGORITHM_COUNT; ++a) {
                add_result_page(app, &comparison.results[a], &input);
            }
            add_comparison_summary(app, &comparison);
            gtk_label_set_text(GTK_LABEL(app->status_label), "Comparison completed successfully.");
        }
        page_comparison_result_destroy(&comparison);
    } else {
        PageSimulationResult result = {0};
        PageAlgorithm algorithm = (PageAlgorithm)choice;
        if (algorithm == PAGE_ALGORITHM_FIFO) status = page_simulate_fifo(&input, &result);
        else if (algorithm == PAGE_ALGORITHM_LRU) status = page_simulate_lru(&input, &result);
        else status = page_simulate_optimal(&input, &result);

        if (status != PAGE_REPLACEMENT_OK) {
            char *message = g_strdup_printf("Simulation failed: %s",
                                            page_replacement_status_name(status));
            show_error(GTK_WINDOW(app->window), message);
            gtk_label_set_text(GTK_LABEL(app->status_label), "Simulation failed.");
            g_free(message);
        } else {
            add_result_page(app, &result, &input);
            gtk_label_set_text(GTK_LABEL(app->status_label), "Simulation completed successfully.");
        }
        page_simulation_result_destroy(&result);
    }

    page_reference_input_destroy(&input);
}

static GtkWidget *make_labeled_entry(GtkWidget *grid, const char *label_text,
                                     const char *placeholder, int row)
{
    GtkWidget *label = gtk_label_new(label_text);
    GtkWidget *entry = gtk_entry_new();
    gtk_label_set_xalign(GTK_LABEL(label), 0.0f);
    gtk_entry_set_placeholder_text(GTK_ENTRY(entry), placeholder);
    gtk_grid_attach(GTK_GRID(grid), label, 0, row, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), entry, 1, row, 1, 1);
    return entry;
}

static void activate(GtkApplication *application, gpointer user_data)
{
    AppWidgets *app = user_data;
    GtkWidget *root;
    GtkWidget *title;
    GtkWidget *input_frame;
    GtkWidget *grid;
    GtkWidget *run_button;
    GtkWidget *results_frame;

    app->window = gtk_application_window_new(application);
    gtk_window_set_title(GTK_WINDOW(app->window), "Page Replacement Simulator");
    gtk_window_set_default_size(GTK_WINDOW(app->window), 900, 650);

    root = gtk_box_new(GTK_ORIENTATION_VERTICAL, 12);
    gtk_container_set_border_width(GTK_CONTAINER(root), 16);
    gtk_container_add(GTK_CONTAINER(app->window), root);

    title = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(title), "<span size='x-large' weight='bold'>Page Replacement Simulator</span>\n<span foreground='gray'>Operating Systems Project</span>");
    gtk_label_set_xalign(GTK_LABEL(title), 0.0f);
    gtk_box_pack_start(GTK_BOX(root), title, FALSE, FALSE, 0);

    input_frame = gtk_frame_new("Simulation Input");
    gtk_box_pack_start(GTK_BOX(root), input_frame, FALSE, FALSE, 0);
    grid = gtk_grid_new();
    gtk_grid_set_row_spacing(GTK_GRID(grid), 8);
    gtk_grid_set_column_spacing(GTK_GRID(grid), 12);
    gtk_container_set_border_width(GTK_CONTAINER(grid), 12);
    gtk_container_add(GTK_CONTAINER(input_frame), grid);

    app->reference_entry = make_labeled_entry(grid, "Page-reference string:",
                                               "Example: 7 0 1 2 0 3 0 4 2 3", 0);
    gtk_entry_set_text(GTK_ENTRY(app->reference_entry), "7 0 1 2 0 3 0 4 2 3");
    app->frames_entry = make_labeled_entry(grid, "Number of frames:", "Example: 3", 1);
    gtk_entry_set_text(GTK_ENTRY(app->frames_entry), "3");

    GtkWidget *algorithm_label = gtk_label_new("Select algorithm:");
    gtk_label_set_xalign(GTK_LABEL(algorithm_label), 0.0f);
    app->algorithm_combo = gtk_combo_box_text_new();
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(app->algorithm_combo), "FIFO");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(app->algorithm_combo), "LRU");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(app->algorithm_combo), "Optimal");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(app->algorithm_combo), "Compare all algorithms");
    gtk_combo_box_set_active(GTK_COMBO_BOX(app->algorithm_combo), 3);
    gtk_grid_attach(GTK_GRID(grid), algorithm_label, 0, 2, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), app->algorithm_combo, 1, 2, 1, 1);

    run_button = gtk_button_new_with_label("Run Simulation");
    gtk_widget_set_halign(run_button, GTK_ALIGN_START);
    gtk_box_pack_start(GTK_BOX(root), run_button, FALSE, FALSE, 0);
    g_signal_connect(run_button, "clicked", G_CALLBACK(on_run_clicked), app);

    results_frame = gtk_frame_new("Simulation Results");
    gtk_box_pack_start(GTK_BOX(root), results_frame, TRUE, TRUE, 0);
    app->notebook = gtk_notebook_new();
    gtk_container_set_border_width(GTK_CONTAINER(app->notebook), 8);
    gtk_container_add(GTK_CONTAINER(results_frame), app->notebook);

    app->status_label = gtk_label_new("Enter the inputs and click Run Simulation.");
    gtk_label_set_xalign(GTK_LABEL(app->status_label), 0.0f);
    gtk_box_pack_start(GTK_BOX(root), app->status_label, FALSE, FALSE, 0);

    gtk_widget_show_all(app->window);
}

int main(int argc, char **argv)
{
    GtkApplication *application;
    AppWidgets app = {0};
    int status;

    application = gtk_application_new("edu.project.pagereplacement", G_APPLICATION_DEFAULT_FLAGS);
    g_signal_connect(application, "activate", G_CALLBACK(activate), &app);
    status = g_application_run(G_APPLICATION(application), argc, argv);
    g_object_unref(application);
    return status;
}
