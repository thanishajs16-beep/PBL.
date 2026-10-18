#include <gtk/gtk.h>
#include <cairo.h>
#include <errno.h>
#include <limits.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "deadlock.h"

typedef struct {
    DeadlockSystem *system;
    GtkWidget *window;
    GtkWidget *status_label;
    GtkWidget *matrix_view;
    GtkWidget *results_view;
    GtkWidget *utilization_view;
    GtkWidget *graph_area;
} GuiState;

typedef enum {
    CELL_ALLOCATION,
    CELL_MAXIMUM,
    CELL_REQUEST
} MatrixCell;

static void refresh_all(GuiState *app);
static void refresh_matrices(GuiState *app);
static void refresh_results(GuiState *app, const char *text);
static void refresh_utilization(GuiState *app);
static void update_system_status(GuiState *app);
static void show_message(GuiState *app, GtkMessageType type, const char *message);
static void edit_matrix_cell(GuiState *app, MatrixCell cell);

static int bounded_process_count(const DeadlockSystem *system) {
    if (system->process_count < 0) return 0;
    if (system->process_count > DEADLOCK_MAX_PROCESSES) return DEADLOCK_MAX_PROCESSES;
    return system->process_count;
}

static int bounded_resource_count(const DeadlockSystem *system) {
    if (system->resource_count < 0) return 0;
    if (system->resource_count > DEADLOCK_MAX_RESOURCES) return DEADLOCK_MAX_RESOURCES;
    return system->resource_count;
}

static int parse_integer(const char *text, int *value) {
    char *copy;
    char *end;
    long parsed;
    int valid = 0;

    if (text == NULL || value == NULL) return 0;
    copy = g_strdup(text);
    g_strstrip(copy);
    if (*copy == '\0') {
        g_free(copy);
        return 0;
    }

    errno = 0;
    parsed = strtol(copy, &end, 10);
    if (errno == 0 && end != copy && *end == '\0' && parsed >= INT_MIN && parsed <= INT_MAX) {
        *value = (int)parsed;
        valid = 1;
    }

    g_free(copy);
    return valid;
}

static void show_message(GuiState *app, GtkMessageType type, const char *message) {
    GtkWidget *dialog = gtk_message_dialog_new(
        app != NULL && app->window != NULL ? GTK_WINDOW(app->window) : NULL,
        GTK_DIALOG_MODAL,
        type,
        GTK_BUTTONS_CLOSE,
        "%s",
        message != NULL ? message : ""
    );
    gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);
}

static GtkWidget *create_output_view(void) {
    GtkWidget *view = gtk_text_view_new();
    gtk_text_view_set_editable(GTK_TEXT_VIEW(view), FALSE);
    gtk_text_view_set_cursor_visible(GTK_TEXT_VIEW(view), FALSE);
    gtk_text_view_set_monospace(GTK_TEXT_VIEW(view), TRUE);
    gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(view), GTK_WRAP_NONE);
    gtk_widget_set_margin_start(view, 10);
    gtk_widget_set_margin_end(view, 10);
    gtk_widget_set_margin_top(view, 10);
    gtk_widget_set_margin_bottom(view, 10);
    return view;
}

static void set_output_text(GtkWidget *view, const char *text) {
    GtkTextBuffer *buffer;
    if (view == NULL) return;
    buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(view));
    gtk_text_buffer_set_text(buffer, text != NULL ? text : "", -1);
}

static GtkWidget *create_process_combo(const DeadlockSystem *system) {
    GtkWidget *combo = gtk_combo_box_text_new();
    int i;
    int count = bounded_process_count(system);

    for (i = 0; i < count; i++) {
        char *id = g_strdup_printf("%d", i);
        char *label = g_strdup_printf("P%d (PID %d)%s", i + 1, system->pid[i], system->active[i] ? "" : " [inactive]");
        gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(combo), id, label);
        g_free(id);
        g_free(label);
    }

    if (count > 0) gtk_combo_box_set_active(GTK_COMBO_BOX(combo), 0);
    return combo;
}

static GtkWidget *create_resource_combo(const DeadlockSystem *system) {
    GtkWidget *combo = gtk_combo_box_text_new();
    int i;
    int count = bounded_resource_count(system);

    for (i = 0; i < count; i++) {
        char *id = g_strdup_printf("%d", i);
        char *label = g_strdup_printf("R%d", i + 1);
        gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(combo), id, label);
        g_free(id);
        g_free(label);
    }

    if (count > 0) gtk_combo_box_set_active(GTK_COMBO_BOX(combo), 0);
    return combo;
}

static int combo_active_index(GtkWidget *combo, int *index) {
    const char *id;
    if (combo == NULL || index == NULL) return 0;
    id = gtk_combo_box_get_active_id(GTK_COMBO_BOX(combo));
    if (id == NULL) return 0;
    return parse_integer(id, index) && *index >= 0;
}

static void append_vector(GString *text, const char *label, const int *values, int count) {
    int i;
    g_string_append_printf(text, "%s: [", label);
    for (i = 0; i < count; i++) {
        g_string_append_printf(text, "%s%d", i == 0 ? "" : ", ", values[i]);
    }
    g_string_append(text, "]\n");
}

static void refresh_matrices(GuiState *app) {
    DeadlockSystem *system = app->system;
    GString *text = g_string_new(NULL);
    int processes = bounded_process_count(system);
    int resources = bounded_resource_count(system);
    int i;
    int j;

    if (processes > 0 && resources > 0) {
        deadlock_recalculate_available(system);
        deadlock_recalculate_need(system);
    }

    g_string_append_printf(text, "Processes: %d    Resource types: %d\n\n", processes, resources);
    append_vector(text, "Available", system->available, resources);
    append_vector(text, "Total", system->total, resources);
    g_string_append(text, "\nAllocation Matrix\n");
    g_string_append(text, "PID   Active   ");
    for (j = 0; j < resources; j++) g_string_append_printf(text, "R%-6d", j + 1);
    g_string_append_c(text, '\n');
    for (i = 0; i < processes; i++) {
        g_string_append_printf(text, "%-5d %-8s", system->pid[i], system->active[i] ? "Yes" : "No");
        for (j = 0; j < resources; j++) g_string_append_printf(text, "%-7d", system->allocation[i][j]);
        g_string_append_c(text, '\n');
    }

    g_string_append(text, "\nMaximum Matrix\n");
    g_string_append(text, "PID   ");
    for (j = 0; j < resources; j++) g_string_append_printf(text, "R%-6d", j + 1);
    g_string_append_c(text, '\n');
    for (i = 0; i < processes; i++) {
        g_string_append_printf(text, "%-5d ", system->pid[i]);
        for (j = 0; j < resources; j++) g_string_append_printf(text, "%-7d", system->maximum[i][j]);
        g_string_append_c(text, '\n');
    }

    g_string_append(text, "\nNeed Matrix\n");
    g_string_append(text, "PID   ");
    for (j = 0; j < resources; j++) g_string_append_printf(text, "R%-6d", j + 1);
    g_string_append_c(text, '\n');
    for (i = 0; i < processes; i++) {
        g_string_append_printf(text, "%-5d ", system->pid[i]);
        for (j = 0; j < resources; j++) g_string_append_printf(text, "%-7d", system->need[i][j]);
        g_string_append_c(text, '\n');
    }

    g_string_append(text, "\nRequest Matrix\n");
    g_string_append(text, "PID   ");
    for (j = 0; j < resources; j++) g_string_append_printf(text, "R%-6d", j + 1);
    g_string_append_c(text, '\n');
    for (i = 0; i < processes; i++) {
        g_string_append_printf(text, "%-5d ", system->pid[i]);
        for (j = 0; j < resources; j++) g_string_append_printf(text, "%-7d", system->request[i][j]);
        g_string_append_c(text, '\n');
    }

    if (processes == 0) g_string_append(text, "\nAdd at least one process to begin.\n");
    if (resources == 0) g_string_append(text, "\nAdd at least one resource type to begin.\n");

    set_output_text(app->matrix_view, text->str);
    g_string_free(text, TRUE);
}

static const char *safety_status_text(DeadlockSafetyStatus status) {
    switch (status) {
        case DEADLOCK_SAFETY_SAFE: return "SAFE";
        case DEADLOCK_SAFETY_UNSAFE: return "UNSAFE";
        case DEADLOCK_SAFETY_INVALID: return "INVALID STATE";
        case DEADLOCK_SAFETY_NOT_CHECKED:
        default: return "NOT CHECKED";
    }
}

static const char *detection_status_text(DeadlockDetectionStatus status) {
    switch (status) {
        case DEADLOCK_DETECTION_NO_DEADLOCK: return "NO DEADLOCK DETECTED";
        case DEADLOCK_DETECTION_FOUND: return "DEADLOCK DETECTED";
        case DEADLOCK_DETECTION_INVALID: return "INVALID STATE";
        case DEADLOCK_DETECTION_NOT_RUN:
        default: return "NOT RUN";
    }
}

static const char *system_status_text(DeadlockSystemStatus status) {
    switch (status) {
        case DEADLOCK_SYSTEM_STATUS_SAFE: return "SAFE";
        case DEADLOCK_SYSTEM_STATUS_UNSAFE: return "UNSAFE";
        case DEADLOCK_SYSTEM_STATUS_DEADLOCKED: return "DEADLOCKED";
        case DEADLOCK_SYSTEM_STATUS_INVALID: return "INVALID STATE";
        case DEADLOCK_SYSTEM_STATUS_UNKNOWN:
        default: return "UNKNOWN";
    }
}

static const char *request_status_text(DeadlockRequestStatus status) {
    switch (status) {
        case DEADLOCK_REQUEST_GRANTED: return "Request granted.";
        case DEADLOCK_REQUEST_WAITING: return "Request cannot be granted now; process must wait.";
        case DEADLOCK_REQUEST_EXCEEDS_NEED: return "Request exceeds the process's remaining declared need.";
        case DEADLOCK_REQUEST_UNSAFE: return "Request denied because it would leave the system unsafe.";
        case DEADLOCK_REQUEST_INVALID:
        default: return "Invalid resource request or system state.";
    }
}

static void append_safe_sequence(GString *text, const BankerResult *result) {
    int i;
    int length;
    if (result == NULL) {
        g_string_append(text, "Safe sequence: unavailable\n");
        return;
    }

    length = result->safe_sequence_length;
    if (length < 0) length = 0;
    if (length > DEADLOCK_MAX_PROCESSES) length = DEADLOCK_MAX_PROCESSES;

    if (length == 0) {
        g_string_append(text, "Safe sequence: none\n");
        return;
    }

    g_string_append(text, "Safe sequence: ");
    for (i = 0; i < length; i++) {
        g_string_append_printf(text, "%sP%d", i == 0 ? "" : " -> ", result->safe_sequence_pids[i]);
    }
    g_string_append_c(text, '\n');
}

static void append_deadlocked_processes(GString *text, const DeadlockDetectionResult *result) {
    int i;
    int count;
    if (result == NULL) {
        g_string_append(text, "Deadlocked processes: unavailable\n");
        return;
    }

    count = result->deadlocked_count;
    if (count < 0) count = 0;
    if (count > DEADLOCK_MAX_PROCESSES) count = DEADLOCK_MAX_PROCESSES;

    if (count == 0) {
        g_string_append(text, "Deadlocked processes: none\n");
        return;
    }

    g_string_append(text, "Deadlocked processes: ");
    for (i = 0; i < count; i++) {
        g_string_append_printf(text, "%sP%d", i == 0 ? "" : ", ", result->deadlocked_pids[i]);
    }
    g_string_append_c(text, '\n');
}

static void refresh_results(GuiState *app, const char *text) {
    set_output_text(app->results_view, text);
}

static void refresh_utilization(GuiState *app) {
    DeadlockSystem *system = app->system;
    GString *text = g_string_new(NULL);
    int resources = bounded_resource_count(system);
    int i;

    g_string_append(text, "Resource Utilization\n\n");
    if (resources == 0) {
        g_string_append(text, "Add resources to calculate utilization.\n");
    } else {
        for (i = 0; i < resources; i++) {
            g_string_append_printf(
                text,
                "R%d: %.2f%% used (%d of %d instances allocated)\n",
                i + 1,
                resource_utilization_percent(system, i),
                system->total[i] - system->available[i],
                system->total[i]
            );
        }
        g_string_append_printf(text, "\nOverall resource utilization: %.2f%%\n", overall_resource_utilization_percent(system));
    }

    set_output_text(app->utilization_view, text->str);
    g_string_free(text, TRUE);
}

static void update_system_status(GuiState *app) {
    DeadlockSystem *system = app->system;
    int processes = bounded_process_count(system);
    int resources = bounded_resource_count(system);

    if (processes == 0 || resources == 0) {
        gtk_label_set_text(GTK_LABEL(app->status_label), "System status: Add processes and resources to begin.");
    } else {
        BankerResult banker_result;
        DeadlockDetectionResult detection_result;
        DeadlockSystemStatus status;
        memset(&banker_result, 0, sizeof(banker_result));
        memset(&detection_result, 0, sizeof(detection_result));
        status = evaluate_system_status(system, &banker_result, &detection_result);
        char *label = g_strdup_printf("System status: %s", system_status_text(status));
        gtk_label_set_text(GTK_LABEL(app->status_label), label);
        g_free(label);
    }
}

static void refresh_graph_size(GuiState *app) {
    int rows = bounded_process_count(app->system);
    int resources = bounded_resource_count(app->system);
    int height;
    if (resources > rows) rows = resources;
    height = 100 + (rows + 1) * 58;
    if (height < 520) height = 520;
    if (height > 1700) height = 1700;
    gtk_widget_set_size_request(app->graph_area, 950, height);
    gtk_widget_queue_draw(app->graph_area);
}

static void refresh_all(GuiState *app) {
    refresh_matrices(app);
    refresh_utilization(app);
    update_system_status(app);
    refresh_graph_size(app);
}

static void add_process_clicked(GtkButton *button, gpointer user_data) {
    GuiState *app = user_data;
    GtkWidget *dialog;
    GtkWidget *content;
    GtkWidget *grid;
    GtkWidget *entry;
    int pid;
    int i;
    int processes;
    int response;
    char *message = NULL;
    (void)button;

    if (bounded_process_count(app->system) >= DEADLOCK_MAX_PROCESSES) {
        show_message(app, GTK_MESSAGE_WARNING, "The maximum number of processes has been reached.");
        return;
    }

    dialog = gtk_dialog_new_with_buttons(
        "Create Process",
        GTK_WINDOW(app->window),
        GTK_DIALOG_MODAL,
        "Cancel", GTK_RESPONSE_CANCEL,
        "Add Process", GTK_RESPONSE_ACCEPT,
        NULL
    );
    content = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    grid = gtk_grid_new();
    gtk_grid_set_row_spacing(GTK_GRID(grid), 8);
    gtk_grid_set_column_spacing(GTK_GRID(grid), 10);
    gtk_widget_set_margin_start(grid, 12);
    gtk_widget_set_margin_end(grid, 12);
    gtk_widget_set_margin_top(grid, 12);
    gtk_widget_set_margin_bottom(grid, 12);
    gtk_grid_attach(GTK_GRID(grid), gtk_label_new("Process ID (PID):"), 0, 0, 1, 1);
    entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(entry), "Enter a unique integer PID");
    gtk_grid_attach(GTK_GRID(grid), entry, 1, 0, 1, 1);
    gtk_container_add(GTK_CONTAINER(content), grid);
    gtk_widget_show_all(dialog);
    response = gtk_dialog_run(GTK_DIALOG(dialog));

    if (response == GTK_RESPONSE_ACCEPT) {
        if (!parse_integer(gtk_entry_get_text(GTK_ENTRY(entry)), &pid)) {
            message = g_strdup("Enter a valid integer PID.");
        } else {
            processes = bounded_process_count(app->system);
            for (i = 0; i < processes; i++) {
                if (app->system->pid[i] == pid) break;
            }
            if (i != processes) {
                message = g_strdup("That PID already exists. Enter a unique PID.");
            } else if (!deadlock_add_process(app->system, pid)) {
                message = g_strdup("The process could not be added. Check the system state and process limit.");
            }
        }
    }

    gtk_widget_destroy(dialog);
    if (message != NULL) {
        show_message(app, GTK_MESSAGE_WARNING, message);
        g_free(message);
    } else if (response == GTK_RESPONSE_ACCEPT) {
        refresh_all(app);
    }
}

static void add_resource_clicked(GtkButton *button, gpointer user_data) {
    GuiState *app = user_data;
    GtkWidget *dialog;
    GtkWidget *content;
    GtkWidget *grid;
    GtkWidget *spin;
    int response;
    int total;
    (void)button;

    if (bounded_resource_count(app->system) >= DEADLOCK_MAX_RESOURCES) {
        show_message(app, GTK_MESSAGE_WARNING, "The maximum number of resource types has been reached.");
        return;
    }

    dialog = gtk_dialog_new_with_buttons(
        "Create Resource Type",
        GTK_WINDOW(app->window),
        GTK_DIALOG_MODAL,
        "Cancel", GTK_RESPONSE_CANCEL,
        "Add Resource", GTK_RESPONSE_ACCEPT,
        NULL
    );
    content = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    grid = gtk_grid_new();
    gtk_grid_set_row_spacing(GTK_GRID(grid), 8);
    gtk_grid_set_column_spacing(GTK_GRID(grid), 10);
    gtk_widget_set_margin_start(grid, 12);
    gtk_widget_set_margin_end(grid, 12);
    gtk_widget_set_margin_top(grid, 12);
    gtk_widget_set_margin_bottom(grid, 12);
    gtk_grid_attach(GTK_GRID(grid), gtk_label_new("Total instances:"), 0, 0, 1, 1);
    spin = gtk_spin_button_new_with_range(0, 100000, 1);
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(spin), 1);
    gtk_grid_attach(GTK_GRID(grid), spin, 1, 0, 1, 1);
    gtk_container_add(GTK_CONTAINER(content), grid);
    gtk_widget_show_all(dialog);
    response = gtk_dialog_run(GTK_DIALOG(dialog));

    total = (int)gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(spin));
    gtk_widget_destroy(dialog);

    if (response == GTK_RESPONSE_ACCEPT) {
        if (!deadlock_add_resource(app->system, total)) {
            show_message(app, GTK_MESSAGE_WARNING, "The resource type could not be added. Check the instance count and resource limit.");
        } else {
            refresh_all(app);
        }
    }
}

static void edit_matrix_cell(GuiState *app, MatrixCell cell) {
    GtkWidget *dialog;
    GtkWidget *content;
    GtkWidget *grid;
    GtkWidget *process_combo;
    GtkWidget *resource_combo;
    GtkWidget *spin;
    GtkWidget *title_label;
    int process_index;
    int resource_index;
    int instances;
    int response;
    int ok = 0;
    const char *title;

    if (bounded_process_count(app->system) == 0 || bounded_resource_count(app->system) == 0) {
        show_message(app, GTK_MESSAGE_WARNING, "Create at least one process and one resource type first.");
        return;
    }

    if (cell == CELL_ALLOCATION) title = "Set Allocation Matrix Value";
    else if (cell == CELL_MAXIMUM) title = "Set Maximum Matrix Value";
    else title = "Set Request Matrix Value";

    dialog = gtk_dialog_new_with_buttons(
        title,
        GTK_WINDOW(app->window),
        GTK_DIALOG_MODAL,
        "Cancel", GTK_RESPONSE_CANCEL,
        "Save Value", GTK_RESPONSE_ACCEPT,
        NULL
    );
    content = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    grid = gtk_grid_new();
    gtk_grid_set_row_spacing(GTK_GRID(grid), 8);
    gtk_grid_set_column_spacing(GTK_GRID(grid), 10);
    gtk_widget_set_margin_start(grid, 12);
    gtk_widget_set_margin_end(grid, 12);
    gtk_widget_set_margin_top(grid, 12);
    gtk_widget_set_margin_bottom(grid, 12);

    title_label = gtk_label_new("Process:");
    gtk_grid_attach(GTK_GRID(grid), title_label, 0, 0, 1, 1);
    process_combo = create_process_combo(app->system);
    gtk_grid_attach(GTK_GRID(grid), process_combo, 1, 0, 1, 1);

    gtk_grid_attach(GTK_GRID(grid), gtk_label_new("Resource:"), 0, 1, 1, 1);
    resource_combo = create_resource_combo(app->system);
    gtk_grid_attach(GTK_GRID(grid), resource_combo, 1, 1, 1, 1);

    gtk_grid_attach(GTK_GRID(grid), gtk_label_new("Instances:"), 0, 2, 1, 1);
    spin = gtk_spin_button_new_with_range(0, 100000, 1);
    gtk_grid_attach(GTK_GRID(grid), spin, 1, 2, 1, 1);

    gtk_container_add(GTK_CONTAINER(content), grid);
    gtk_widget_show_all(dialog);
    response = gtk_dialog_run(GTK_DIALOG(dialog));

    if (response == GTK_RESPONSE_ACCEPT &&
        combo_active_index(process_combo, &process_index) &&
        combo_active_index(resource_combo, &resource_index)) {
        instances = (int)gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(spin));
        if (cell == CELL_ALLOCATION) {
            ok = deadlock_set_allocation(app->system, process_index, resource_index, instances);
        } else if (cell == CELL_MAXIMUM) {
            ok = deadlock_set_maximum(app->system, process_index, resource_index, instances);
        } else {
            ok = deadlock_set_request(app->system, process_index, resource_index, instances);
        }
    }

    gtk_widget_destroy(dialog);

    if (response == GTK_RESPONSE_ACCEPT) {
        if (!ok) {
            show_message(app, GTK_MESSAGE_WARNING, "The value could not be saved. Check the process, resource, allocation, and maximum requirements.");
        } else {
            refresh_all(app);
        }
    }
}

static void set_allocation_clicked(GtkButton *button, gpointer user_data) {
    (void)button;
    edit_matrix_cell((GuiState *)user_data, CELL_ALLOCATION);
}

static void set_maximum_clicked(GtkButton *button, gpointer user_data) {
    (void)button;
    edit_matrix_cell((GuiState *)user_data, CELL_MAXIMUM);
}

static void set_request_clicked(GtkButton *button, gpointer user_data) {
    (void)button;
    edit_matrix_cell((GuiState *)user_data, CELL_REQUEST);
}

static void submit_resource_request_clicked(GtkButton *button, gpointer user_data) {
    GuiState *app = user_data;
    GtkWidget *dialog;
    GtkWidget *content;
    GtkWidget *grid;
    GtkWidget *process_combo;
    GtkWidget *spins[DEADLOCK_MAX_RESOURCES];
    int request_vector[DEADLOCK_MAX_RESOURCES] = {0};
    int process_index;
    int resources = bounded_resource_count(app->system);
    int i;
    int response;
    DeadlockRequestStatus status;
    BankerResult result;
    GString *output;
    (void)button;

    if (bounded_process_count(app->system) == 0 || resources == 0) {
        show_message(app, GTK_MESSAGE_WARNING, "Create at least one process and one resource type first.");
        return;
    }

    dialog = gtk_dialog_new_with_buttons(
        "Request Resources",
        GTK_WINDOW(app->window),
        GTK_DIALOG_MODAL,
        "Cancel", GTK_RESPONSE_CANCEL,
        "Submit Request", GTK_RESPONSE_ACCEPT,
        NULL
    );
    content = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    grid = gtk_grid_new();
    gtk_grid_set_row_spacing(GTK_GRID(grid), 8);
    gtk_grid_set_column_spacing(GTK_GRID(grid), 10);
    gtk_widget_set_margin_start(grid, 12);
    gtk_widget_set_margin_end(grid, 12);
    gtk_widget_set_margin_top(grid, 12);
    gtk_widget_set_margin_bottom(grid, 12);

    gtk_grid_attach(GTK_GRID(grid), gtk_label_new("Process:"), 0, 0, 1, 1);
    process_combo = create_process_combo(app->system);
    gtk_grid_attach(GTK_GRID(grid), process_combo, 1, 0, 1, 1);

    for (i = 0; i < resources; i++) {
        char *label = g_strdup_printf("Request for R%d:", i + 1);
        gtk_grid_attach(GTK_GRID(grid), gtk_label_new(label), 0, i + 1, 1, 1);
        g_free(label);
        spins[i] = gtk_spin_button_new_with_range(0, 100000, 1);
        gtk_grid_attach(GTK_GRID(grid), spins[i], 1, i + 1, 1, 1);
    }

    gtk_container_add(GTK_CONTAINER(content), grid);
    gtk_widget_show_all(dialog);
    response = gtk_dialog_run(GTK_DIALOG(dialog));

    if (response == GTK_RESPONSE_ACCEPT && combo_active_index(process_combo, &process_index)) {
        for (i = 0; i < resources; i++) {
            request_vector[i] = (int)gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(spins[i]));
        }
        memset(&result, 0, sizeof(result));
        status = request_resources(app->system, process_index, request_vector, &result);
        output = g_string_new("Resource Request Result\n\n");
        g_string_append_printf(output, "%s\n", request_status_text(status));
        if (status == DEADLOCK_REQUEST_GRANTED) append_safe_sequence(output, &result);
        refresh_results(app, output->str);
        g_string_free(output, TRUE);
    }

    gtk_widget_destroy(dialog);
    if (response == GTK_RESPONSE_ACCEPT) refresh_all(app);
}

static void run_bankers_clicked(GtkButton *button, gpointer user_data) {
    GuiState *app = user_data;
    BankerResult result;
    DeadlockSafetyStatus status;
    GString *output;
    (void)button;

    memset(&result, 0, sizeof(result));
    status = run_bankers_algorithm(app->system, &result);
    output = g_string_new("Banker's Algorithm\n\n");
    g_string_append_printf(output, "Safety status: %s\n", safety_status_text(status));
    if (status == DEADLOCK_SAFETY_SAFE) append_safe_sequence(output, &result);
    else g_string_append(output, "No safe sequence was returned for the current state.\n");
    refresh_results(app, output->str);
    g_string_free(output, TRUE);
    refresh_all(app);
}

static void detect_deadlock_clicked(GtkButton *button, gpointer user_data) {
    GuiState *app = user_data;
    DeadlockDetectionResult result;
    DeadlockDetectionStatus status;
    GString *output;
    (void)button;

    memset(&result, 0, sizeof(result));
    status = detect_deadlock(app->system, &result);
    output = g_string_new("Deadlock Detection\n\n");
    g_string_append_printf(output, "Detection result: %s\n", detection_status_text(status));
    if (status == DEADLOCK_DETECTION_FOUND || status == DEADLOCK_DETECTION_NO_DEADLOCK) {
        append_deadlocked_processes(output, &result);
    }
    refresh_results(app, output->str);
    g_string_free(output, TRUE);
    refresh_all(app);
}

static void recover_from_deadlock_clicked(GtkButton *button, gpointer user_data) {
    GuiState *app = user_data;
    GtkWidget *dialog;
    GtkWidget *content;
    GtkWidget *grid;
    GtkWidget *process_combo;
    GtkWidget *strategy_combo;
    int process_index;
    int strategy_index;
    int response;
    int ok;
    GString *output;
    (void)button;

    if (bounded_process_count(app->system) == 0) {
        show_message(app, GTK_MESSAGE_WARNING, "Create at least one process first.");
        return;
    }

    dialog = gtk_dialog_new_with_buttons(
        "Recover from Deadlock",
        GTK_WINDOW(app->window),
        GTK_DIALOG_MODAL,
        "Cancel", GTK_RESPONSE_CANCEL,
        "Recover", GTK_RESPONSE_ACCEPT,
        NULL
    );
    content = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    grid = gtk_grid_new();
    gtk_grid_set_row_spacing(GTK_GRID(grid), 8);
    gtk_grid_set_column_spacing(GTK_GRID(grid), 10);
    gtk_widget_set_margin_start(grid, 12);
    gtk_widget_set_margin_end(grid, 12);
    gtk_widget_set_margin_top(grid, 12);
    gtk_widget_set_margin_bottom(grid, 12);

    gtk_grid_attach(GTK_GRID(grid), gtk_label_new("Process:"), 0, 0, 1, 1);
    process_combo = create_process_combo(app->system);
    gtk_grid_attach(GTK_GRID(grid), process_combo, 1, 0, 1, 1);

    gtk_grid_attach(GTK_GRID(grid), gtk_label_new("Recovery strategy:"), 0, 1, 1, 1);
    strategy_combo = gtk_combo_box_text_new();
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(strategy_combo), "0", "Preempt and release allocated resources");
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(strategy_combo), "1", "Terminate process and release resources");
    gtk_combo_box_set_active(GTK_COMBO_BOX(strategy_combo), 0);
    gtk_grid_attach(GTK_GRID(grid), strategy_combo, 1, 1, 1, 1);

    gtk_container_add(GTK_CONTAINER(content), grid);
    gtk_widget_show_all(dialog);
    response = gtk_dialog_run(GTK_DIALOG(dialog));

    if (response == GTK_RESPONSE_ACCEPT &&
        combo_active_index(process_combo, &process_index) &&
        combo_active_index(strategy_combo, &strategy_index)) {
        DeadlockRecoveryStrategy strategy = strategy_index == 0
            ? DEADLOCK_RECOVERY_PREEMPT_RESOURCES
            : DEADLOCK_RECOVERY_TERMINATE_PROCESS;
        ok = recover_from_deadlock(app->system, process_index, strategy);
        output = g_string_new("Deadlock Recovery\n\n");
        g_string_append_printf(output, "%s\n", ok ? "Recovery operation completed." : "Recovery operation failed.");
        refresh_results(app, output->str);
        g_string_free(output, TRUE);
    }

    gtk_widget_destroy(dialog);
    if (response == GTK_RESPONSE_ACCEPT) refresh_all(app);
}

static void system_status_clicked(GtkButton *button, gpointer user_data) {
    GuiState *app = user_data;
    BankerResult banker_result;
    DeadlockDetectionResult detection_result;
    DeadlockSystemStatus status;
    GString *output;
    (void)button;

    memset(&banker_result, 0, sizeof(banker_result));
    memset(&detection_result, 0, sizeof(detection_result));
    status = evaluate_system_status(app->system, &banker_result, &detection_result);

    output = g_string_new("System Status\n\n");
    g_string_append_printf(output, "Current state: %s\n", system_status_text(status));
    if (status == DEADLOCK_SYSTEM_STATUS_SAFE) append_safe_sequence(output, &banker_result);
    if (status == DEADLOCK_SYSTEM_STATUS_DEADLOCKED || status == DEADLOCK_SYSTEM_STATUS_UNSAFE) {
        append_deadlocked_processes(output, &detection_result);
    }
    refresh_results(app, output->str);
    g_string_free(output, TRUE);
    refresh_all(app);
}

static void refresh_clicked(GtkButton *button, gpointer user_data) {
    (void)button;
    refresh_all((GuiState *)user_data);
}

static void draw_centered_text(cairo_t *cr, const char *text, double cx, double cy) {
    cairo_text_extents_t extents;
    cairo_text_extents(cr, text, &extents);
    cairo_move_to(cr, cx - extents.width / 2.0 - extents.x_bearing, cy - extents.height / 2.0 - extents.y_bearing);
    cairo_show_text(cr, text);
}

static void graph_node_position(const DeadlockSystem *system, RAGNodeType type, int index, double width, double height, double *x, double *y) {
    int count = type == RAG_NODE_PROCESS ? bounded_process_count(system) : bounded_resource_count(system);
    double top = 58.0;
    double bottom = 35.0;
    double usable = height - top - bottom;

    if (count < 1) count = 1;
    if (usable < 1.0) usable = 1.0;

    *x = type == RAG_NODE_PROCESS ? width * 0.25 : width * 0.75;
    *y = top + usable * ((double)index + 1.0) / ((double)count + 1.0);
}

static void draw_arrow(cairo_t *cr, double x1, double y1, double x2, double y2, gboolean request_edge, int instances) {
    double dx = x2 - x1;
    double dy = y2 - y1;
    double length = sqrt(dx * dx + dy * dy);
    double ux;
    double uy;
    double start_offset = 26.0;
    double end_offset = 30.0;
    double sx;
    double sy;
    double ex;
    double ey;
    double angle;
    double arrow_length = 10.0;
    double arrow_angle = 0.48;
    double midx;
    double midy;
    char label[32];

    if (length < 1.0) return;
    ux = dx / length;
    uy = dy / length;
    sx = x1 + ux * start_offset;
    sy = y1 + uy * start_offset;
    ex = x2 - ux * end_offset;
    ey = y2 - uy * end_offset;

    if (request_edge) cairo_set_source_rgb(cr, 0.78, 0.20, 0.20);
    else cairo_set_source_rgb(cr, 0.12, 0.36, 0.70);
    cairo_set_line_width(cr, 2.2);
    cairo_move_to(cr, sx, sy);
    cairo_line_to(cr, ex, ey);
    cairo_stroke(cr);

    angle = atan2(ey - sy, ex - sx);
    cairo_move_to(cr, ex, ey);
    cairo_line_to(cr, ex - arrow_length * cos(angle - arrow_angle), ey - arrow_length * sin(angle - arrow_angle));
    cairo_line_to(cr, ex - arrow_length * cos(angle + arrow_angle), ey - arrow_length * sin(angle + arrow_angle));
    cairo_close_path(cr);
    cairo_fill(cr);

    midx = (sx + ex) / 2.0;
    midy = (sy + ey) / 2.0 - 6.0;
    if (instances > 0) {
        g_snprintf(label, sizeof(label), "x%d", instances);
        cairo_set_source_rgb(cr, 0.12, 0.12, 0.12);
        cairo_select_font_face(cr, "Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
        cairo_set_font_size(cr, 12.0);
        draw_centered_text(cr, label, midx, midy);
    }
}

static gboolean draw_resource_graph(GtkWidget *widget, cairo_t *cr, gpointer user_data) {
    GuiState *app = user_data;
    DeadlockSystem *system = app->system;
    GtkAllocation allocation;
    double width;
    double height;
    int processes = bounded_process_count(system);
    int resources = bounded_resource_count(system);
    int i;
    ResourceAllocationGraph graph;
    int edge_count;

    gtk_widget_get_allocation(widget, &allocation);
    width = allocation.width;
    height = allocation.height;
    if (width < 1.0) width = 950.0;
    if (height < 1.0) height = 520.0;

    cairo_set_source_rgb(cr, 1.0, 1.0, 1.0);
    cairo_paint(cr);
    cairo_select_font_face(cr, "Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
    cairo_set_font_size(cr, 15.0);
    cairo_set_source_rgb(cr, 0.20, 0.20, 0.20);
    cairo_move_to(cr, 35.0, 28.0);
    cairo_show_text(cr, "Processes");
    cairo_move_to(cr, width * 0.70, 28.0);
    cairo_show_text(cr, "Resource Types");

    if (processes == 0 || resources == 0) {
        cairo_set_font_size(cr, 17.0);
        cairo_move_to(cr, 35.0, 75.0);
        cairo_show_text(cr, "Create processes and resource types to display the Resource Allocation Graph.");
        return FALSE;
    }

    memset(&graph, 0, sizeof(graph));
    build_resource_allocation_graph(system, &graph);
    edge_count = graph.edge_count;
    if (edge_count < 0) edge_count = 0;
    if (edge_count > DEADLOCK_MAX_GRAPH_EDGES) edge_count = DEADLOCK_MAX_GRAPH_EDGES;

    for (i = 0; i < edge_count; i++) {
        ResourceGraphEdge *edge = &graph.edges[i];
        double x1;
        double y1;
        double x2;
        double y2;
        int from_limit;
        int to_limit;

        from_limit = edge->from_type == RAG_NODE_PROCESS ? processes : resources;
        to_limit = edge->to_type == RAG_NODE_PROCESS ? processes : resources;
        if (edge->from_index < 0 || edge->from_index >= from_limit || edge->to_index < 0 || edge->to_index >= to_limit) continue;

        graph_node_position(system, edge->from_type, edge->from_index, width, height, &x1, &y1);
        graph_node_position(system, edge->to_type, edge->to_index, width, height, &x2, &y2);
        draw_arrow(cr, x1, y1, x2, y2, edge->edge_type == RAG_EDGE_REQUEST, edge->instances);
    }

    for (i = 0; i < processes; i++) {
        double x;
        double y;
        char label[40];
        graph_node_position(system, RAG_NODE_PROCESS, i, width, height, &x, &y);
        if (system->active[i]) cairo_set_source_rgb(cr, 0.79, 0.88, 0.98);
        else cairo_set_source_rgb(cr, 0.84, 0.84, 0.84);
        cairo_arc(cr, x, y, 25.0, 0.0, 2.0 * G_PI);
        cairo_fill_preserve(cr);
        cairo_set_source_rgb(cr, 0.17, 0.31, 0.47);
        cairo_set_line_width(cr, 1.5);
        cairo_stroke(cr);
        cairo_set_source_rgb(cr, 0.08, 0.12, 0.18);
        cairo_set_font_size(cr, 13.0);
        g_snprintf(label, sizeof(label), "P%d", system->pid[i]);
        draw_centered_text(cr, label, x, y);
    }

    for (i = 0; i < resources; i++) {
        double x;
        double y;
        char label[40];
        char count_label[40];
        graph_node_position(system, RAG_NODE_RESOURCE, i, width, height, &x, &y);
        cairo_rectangle(cr, x - 39.0, y - 22.0, 78.0, 44.0);
        cairo_set_source_rgb(cr, 0.99, 0.88, 0.68);
        cairo_fill_preserve(cr);
        cairo_set_source_rgb(cr, 0.47, 0.33, 0.15);
        cairo_set_line_width(cr, 1.5);
        cairo_stroke(cr);
        cairo_set_source_rgb(cr, 0.15, 0.12, 0.08);
        cairo_set_font_size(cr, 13.0);
        g_snprintf(label, sizeof(label), "R%d", i + 1);
        draw_centered_text(cr, label, x, y - 4.0);
        cairo_set_font_size(cr, 10.0);
        g_snprintf(count_label, sizeof(count_label), "Total: %d", system->total[i]);
        draw_centered_text(cr, count_label, x, y + 12.0);
    }

    cairo_set_font_size(cr, 12.0);
    cairo_set_source_rgb(cr, 0.12, 0.36, 0.70);
    cairo_move_to(cr, 35.0, height - 15.0);
    cairo_show_text(cr, "Blue: allocation edge");
    cairo_set_source_rgb(cr, 0.78, 0.20, 0.20);
    cairo_move_to(cr, 220.0, height - 15.0);
    cairo_show_text(cr, "Red: request edge");
    return FALSE;
}

static void add_button(GtkGrid *grid, const char *label, int column, int row, GCallback callback, GuiState *app) {
    GtkWidget *button = gtk_button_new_with_label(label);
    gtk_widget_set_hexpand(button, TRUE);
    gtk_widget_set_size_request(button, 150, 36);
    g_signal_connect(button, "clicked", callback, app);
    gtk_grid_attach(grid, button, column, row, 1, 1);
}

static GtkWidget *create_scrolled_page(GtkWidget *child) {
    GtkWidget *scroll = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_container_add(GTK_CONTAINER(scroll), child);
    return scroll;
}

void gui_run(DeadlockSystem *system) {
    GuiState app;
    GtkWidget *root;
    GtkWidget *title;
    GtkWidget *controls_frame;
    GtkWidget *controls_grid;
    GtkWidget *notebook;
    GtkWidget *matrix_scroll;
    GtkWidget *results_scroll;
    GtkWidget *graph_scroll;
    GtkWidget *utilization_scroll;
    GtkWidget *graph_page;
    GtkWidget *legend;
    GtkWidget *status_box;
    static DeadlockSystem fallback_system;

    memset(&app, 0, sizeof(app));
    if (system == NULL) {
        deadlock_system_init(&fallback_system);
        system = &fallback_system;
    }
    app.system = system;

    gtk_init(NULL, NULL);

    app.window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(app.window), "Deadlock Simulation System");
    gtk_window_set_default_size(GTK_WINDOW(app.window), 1180, 820);
    gtk_window_set_position(GTK_WINDOW(app.window), GTK_WIN_POS_CENTER);
    g_signal_connect(app.window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    root = gtk_box_new(GTK_ORIENTATION_VERTICAL, 8);
    gtk_widget_set_margin_start(root, 12);
    gtk_widget_set_margin_end(root, 12);
    gtk_widget_set_margin_top(root, 10);
    gtk_widget_set_margin_bottom(root, 10);
    gtk_container_add(GTK_CONTAINER(app.window), root);

    title = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(title), "<span size='xx-large' weight='bold'>Deadlock Simulation System</span>");
    gtk_widget_set_halign(title, GTK_ALIGN_START);
    gtk_box_pack_start(GTK_BOX(root), title, FALSE, FALSE, 2);

    controls_frame = gtk_frame_new("System Operations");
    gtk_box_pack_start(GTK_BOX(root), controls_frame, FALSE, FALSE, 2);
    controls_grid = gtk_grid_new();
    gtk_grid_set_row_spacing(GTK_GRID(controls_grid), 6);
    gtk_grid_set_column_spacing(GTK_GRID(controls_grid), 6);
    gtk_widget_set_margin_start(controls_grid, 8);
    gtk_widget_set_margin_end(controls_grid, 8);
    gtk_widget_set_margin_top(controls_grid, 8);
    gtk_widget_set_margin_bottom(controls_grid, 8);
    gtk_container_add(GTK_CONTAINER(controls_frame), controls_grid);

    add_button(GTK_GRID(controls_grid), "Create Process", 0, 0, G_CALLBACK(add_process_clicked), &app);
    add_button(GTK_GRID(controls_grid), "Create Resource", 1, 0, G_CALLBACK(add_resource_clicked), &app);
    add_button(GTK_GRID(controls_grid), "Set Allocation", 2, 0, G_CALLBACK(set_allocation_clicked), &app);
    add_button(GTK_GRID(controls_grid), "Set Maximum", 0, 1, G_CALLBACK(set_maximum_clicked), &app);
    add_button(GTK_GRID(controls_grid), "Set Request", 1, 1, G_CALLBACK(set_request_clicked), &app);
    add_button(GTK_GRID(controls_grid), "Submit Resource Request", 2, 1, G_CALLBACK(submit_resource_request_clicked), &app);
    add_button(GTK_GRID(controls_grid), "Run Banker's Algorithm", 0, 2, G_CALLBACK(run_bankers_clicked), &app);
    add_button(GTK_GRID(controls_grid), "Detect Deadlock", 1, 2, G_CALLBACK(detect_deadlock_clicked), &app);
    add_button(GTK_GRID(controls_grid), "Recover from Deadlock", 2, 2, G_CALLBACK(recover_from_deadlock_clicked), &app);
    add_button(GTK_GRID(controls_grid), "System Status", 0, 3, G_CALLBACK(system_status_clicked), &app);
    add_button(GTK_GRID(controls_grid), "Refresh Display", 1, 3, G_CALLBACK(refresh_clicked), &app);

    notebook = gtk_notebook_new();
    gtk_widget_set_vexpand(notebook, TRUE);
    gtk_box_pack_start(GTK_BOX(root), notebook, TRUE, TRUE, 2);

    app.matrix_view = create_output_view();
    matrix_scroll = create_scrolled_page(app.matrix_view);
    gtk_notebook_append_page(GTK_NOTEBOOK(notebook), matrix_scroll, gtk_label_new("System Matrices"));

    app.results_view = create_output_view();
    results_scroll = create_scrolled_page(app.results_view);
    gtk_notebook_append_page(GTK_NOTEBOOK(notebook), results_scroll, gtk_label_new("Algorithm Results"));

    app.graph_area = gtk_drawing_area_new();
    gtk_widget_set_size_request(app.graph_area, 950, 520);
    g_signal_connect(app.graph_area, "draw", G_CALLBACK(draw_resource_graph), &app);
    graph_scroll = create_scrolled_page(app.graph_area);
    graph_page = gtk_box_new(GTK_ORIENTATION_VERTICAL, 4);
    legend = gtk_label_new("Process nodes are circles. Resource types are rectangles. Blue edges indicate allocations; red edges indicate requests.");
    gtk_label_set_xalign(GTK_LABEL(legend), 0.0f);
    gtk_box_pack_start(GTK_BOX(graph_page), legend, FALSE, FALSE, 5);
    gtk_box_pack_start(GTK_BOX(graph_page), graph_scroll, TRUE, TRUE, 0);
    gtk_notebook_append_page(GTK_NOTEBOOK(notebook), graph_page, gtk_label_new("Resource Allocation Graph"));

    app.utilization_view = create_output_view();
    utilization_scroll = create_scrolled_page(app.utilization_view);
    gtk_notebook_append_page(GTK_NOTEBOOK(notebook), utilization_scroll, gtk_label_new("Resource Utilization"));

    status_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);
    app.status_label = gtk_label_new("System status: Add processes and resources to begin.");
    gtk_label_set_xalign(GTK_LABEL(app.status_label), 0.0f);
    gtk_widget_set_margin_start(app.status_label, 4);
    gtk_box_pack_start(GTK_BOX(status_box), app.status_label, TRUE, TRUE, 2);
    gtk_box_pack_start(GTK_BOX(root), status_box, FALSE, FALSE, 2);

    set_output_text(app.results_view, "Select an operation to view its results here.");
    refresh_all(&app);
    gtk_widget_show_all(app.window);
    gtk_main();
}
