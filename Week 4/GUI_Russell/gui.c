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
    GtkWidget *window, *status_label, *matrix_view, *results_view;
    GtkWidget *utilization_view, *graph_area;
} GuiState;

typedef enum { CELL_ALLOCATION, CELL_MAXIMUM, CELL_REQUEST } MatrixCell;

static void refresh_all(GuiState *app);
static void refresh_matrices(GuiState *app);
static void refresh_results(GuiState *app, const char *text);
static void edit_matrix_cell(GuiState *app, MatrixCell cell);

static int process_count(const DeadlockSystem *s) {
    return s->process_count < 0 ? 0 :
        s->process_count > DEADLOCK_MAX_PROCESSES ? DEADLOCK_MAX_PROCESSES : s->process_count;
}

static int resource_count(const DeadlockSystem *s) {
    return s->resource_count < 0 ? 0 :
        s->resource_count > DEADLOCK_MAX_RESOURCES ? DEADLOCK_MAX_RESOURCES : s->resource_count;
}

static int parse_integer(const char *text, int *value) {
    char *copy, *end;
    long parsed;
    if (!text || !value) return 0;
    copy = g_strdup(text);
    g_strstrip(copy);
    if (!*copy) {
        g_free(copy);
        return 0;
    }
    errno = 0;
    parsed = strtol(copy, &end, 10);
    int valid = !errno && end != copy && !*end && parsed >= INT_MIN && parsed <= INT_MAX;
    if (valid) *value = (int)parsed;
    g_free(copy);
    return valid;
}

static void show_message(GuiState *app, GtkMessageType type, const char *message) {
    GtkWidget *dialog = gtk_message_dialog_new(
        app && app->window ? GTK_WINDOW(app->window) : NULL, GTK_DIALOG_MODAL,
        type, GTK_BUTTONS_CLOSE, "%s", message ? message : "");
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
    if (view)
        gtk_text_buffer_set_text(gtk_text_view_get_buffer(GTK_TEXT_VIEW(view)),
                                 text ? text : "", -1);
}

static GtkWidget *create_index_combo(const DeadlockSystem *s, gboolean processes) {
    GtkWidget *combo = gtk_combo_box_text_new();
    int count = processes ? process_count(s) : resource_count(s);
    for (int i = 0; i < count; i++) {
        char *id = g_strdup_printf("%d", i);
        char *label = processes
            ? g_strdup_printf("P%d (PID %d)%s", i + 1, s->pid[i], s->active[i] ? "" : " [inactive]")
            : g_strdup_printf("R%d", i + 1);
        gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(combo), id, label);
        g_free(id);
        g_free(label);
    }
    if (count) gtk_combo_box_set_active(GTK_COMBO_BOX(combo), 0);
    return combo;
}

static int combo_active_index(GtkWidget *combo, int *index) {
    const char *id = combo ? gtk_combo_box_get_active_id(GTK_COMBO_BOX(combo)) : NULL;
    return id && index && parse_integer(id, index) && *index >= 0;
}

static void append_vector(GString *text, const char *label, const int *values, int count) {
    g_string_append_printf(text, "%s: [", label);
    for (int i = 0; i < count; i++)
        g_string_append_printf(text, "%s%d", i ? ", " : "", values[i]);
    g_string_append(text, "]\n");
}

static void append_matrix(GString *text, const char *title, const DeadlockSystem *s,
                          const int matrix[][DEADLOCK_MAX_RESOURCES], gboolean show_active) {
    int processes = process_count(s), resources = resource_count(s);
    g_string_append_printf(text, "\n%s\nPID   ", title);
    if (show_active) g_string_append(text, "Active   ");
    for (int r = 0; r < resources; r++)
        g_string_append_printf(text, "R%-6d", r + 1);
    g_string_append_c(text, '\n');
    for (int p = 0; p < processes; p++) {
        if (show_active)
            g_string_append_printf(text, "%-5d %-8s", s->pid[p], s->active[p] ? "Yes" : "No");
        else
            g_string_append_printf(text, "%-5d ", s->pid[p]);
        for (int r = 0; r < resources; r++)
            g_string_append_printf(text, "%-7d", matrix[p][r]);
        g_string_append_c(text, '\n');
    }
}

static void refresh_matrices(GuiState *app) {
    DeadlockSystem *s = app->system;
    int processes = process_count(s), resources = resource_count(s);
    GString *text = g_string_new(NULL);
    if (processes && resources) {
        deadlock_recalculate_available(s);
        deadlock_recalculate_need(s);
    }
    g_string_append_printf(text, "Processes: %d    Resource types: %d\n\n", processes, resources);
    append_vector(text, "Available", s->available, resources);
    append_vector(text, "Total", s->total, resources);
    append_matrix(text, "Allocation Matrix", s, s->allocation, TRUE);
    append_matrix(text, "Maximum Matrix", s, s->maximum, FALSE);
    append_matrix(text, "Need Matrix", s, s->need, FALSE);
    append_matrix(text, "Request Matrix", s, s->request, FALSE);
    if (!processes) g_string_append(text, "\nAdd at least one process to begin.\n");
    if (!resources) g_string_append(text, "\nAdd at least one resource type to begin.\n");
    set_output_text(app->matrix_view, text->str);
    g_string_free(text, TRUE);
}

static const char *safety_status_text(DeadlockSafetyStatus status) {
    switch (status) {
        case DEADLOCK_SAFETY_SAFE: return "SAFE";
        case DEADLOCK_SAFETY_UNSAFE: return "UNSAFE";
        case DEADLOCK_SAFETY_INVALID: return "INVALID STATE";
        default: return "NOT CHECKED";
    }
}

static const char *detection_status_text(DeadlockDetectionStatus status) {
    switch (status) {
        case DEADLOCK_DETECTION_NO_DEADLOCK: return "NO DEADLOCK DETECTED";
        case DEADLOCK_DETECTION_FOUND: return "DEADLOCK DETECTED";
        case DEADLOCK_DETECTION_INVALID: return "INVALID STATE";
        default: return "NOT RUN";
    }
}

static const char *system_status_text(DeadlockSystemStatus status) {
    switch (status) {
        case DEADLOCK_SYSTEM_STATUS_SAFE: return "SAFE";
        case DEADLOCK_SYSTEM_STATUS_UNSAFE: return "UNSAFE";
        case DEADLOCK_SYSTEM_STATUS_DEADLOCKED: return "DEADLOCKED";
        case DEADLOCK_SYSTEM_STATUS_INVALID: return "INVALID STATE";
        default: return "UNKNOWN";
    }
}

static const char *request_status_text(DeadlockRequestStatus status) {
    switch (status) {
        case DEADLOCK_REQUEST_GRANTED: return "Request granted.";
        case DEADLOCK_REQUEST_WAITING: return "Request cannot be granted now; process must wait.";
        case DEADLOCK_REQUEST_EXCEEDS_NEED: return "Request exceeds the process's remaining declared need.";
        case DEADLOCK_REQUEST_UNSAFE: return "Request denied because it would leave the system unsafe.";
        default: return "Invalid resource request or system state.";
    }
}

static void append_safe_sequence(GString *text, const BankerResult *result) {
    int count = result ? result->safe_sequence_length : 0;
    if (!result) {
        g_string_append(text, "Safe sequence: unavailable\n");
        return;
    }
    if (count < 0) count = 0;
    if (count > DEADLOCK_MAX_PROCESSES) count = DEADLOCK_MAX_PROCESSES;
    if (!count) {
        g_string_append(text, "Safe sequence: none\n");
        return;
    }
    g_string_append(text, "Safe sequence: ");
    for (int i = 0; i < count; i++)
        g_string_append_printf(text, "%sP%d", i ? " -> " : "", result->safe_sequence_pids[i]);
    g_string_append_c(text, '\n');
}

static void append_deadlocked_processes(GString *text, const DeadlockDetectionResult *result) {
    int count = result ? result->deadlocked_count : 0;
    if (!result) {
        g_string_append(text, "Deadlocked processes: unavailable\n");
        return;
    }
    if (count < 0) count = 0;
    if (count > DEADLOCK_MAX_PROCESSES) count = DEADLOCK_MAX_PROCESSES;
    if (!count) {
        g_string_append(text, "Deadlocked processes: none\n");
        return;
    }
    g_string_append(text, "Deadlocked processes: ");
    for (int i = 0; i < count; i++)
        g_string_append_printf(text, "%sP%d", i ? ", " : "", result->deadlocked_pids[i]);
    g_string_append_c(text, '\n');
}

static void refresh_results(GuiState *app, const char *text) {
    set_output_text(app->results_view, text);
}

static void refresh_utilization(GuiState *app) {
    DeadlockSystem *s = app->system;
    int resources = resource_count(s);
    GString *text = g_string_new("Resource Utilization\n\n");
    if (!resources) {
        g_string_append(text, "Add resources to calculate utilization.\n");
    } else {
        for (int r = 0; r < resources; r++)
            g_string_append_printf(text, "R%d: %.2f%% used (%d of %d instances allocated)\n",
                r + 1, resource_utilization_percent(s, r),
                s->total[r] - s->available[r], s->total[r]);
        g_string_append_printf(text, "\nOverall resource utilization: %.2f%%\n",
                               overall_resource_utilization_percent(s));
    }
    set_output_text(app->utilization_view, text->str);
    g_string_free(text, TRUE);
}

static void update_system_status(GuiState *app) {
    DeadlockSystem *s = app->system;
    if (!process_count(s) || !resource_count(s)) {
        gtk_label_set_text(GTK_LABEL(app->status_label),
                           "System status: Add processes and resources to begin.");
        return;
    }
    BankerResult banker = {0};
    DeadlockDetectionResult detection = {0};
    DeadlockSystemStatus status = evaluate_system_status(s, &banker, &detection);
    char *label = g_strdup_printf("System status: %s", system_status_text(status));
    gtk_label_set_text(GTK_LABEL(app->status_label), label);
    g_free(label);
}

static void refresh_graph_size(GuiState *app) {
    int rows = MAX(process_count(app->system), resource_count(app->system));
    int height = 100 + (rows + 1) * 58;
    gtk_widget_set_size_request(app->graph_area, 950, CLAMP(height, 520, 1700));
    gtk_widget_queue_draw(app->graph_area);
}

static void refresh_all(GuiState *app) {
    refresh_matrices(app);
    refresh_utilization(app);
    update_system_status(app);
    refresh_graph_size(app);
}

static GtkWidget *create_form_dialog(GuiState *app, const char *title,
                                     const char *action, GtkWidget **grid_out) {
    GtkWidget *dialog = gtk_dialog_new_with_buttons(title, GTK_WINDOW(app->window),
        GTK_DIALOG_MODAL, "Cancel", GTK_RESPONSE_CANCEL, action, GTK_RESPONSE_ACCEPT, NULL);
    GtkWidget *grid = gtk_grid_new();
    gtk_grid_set_row_spacing(GTK_GRID(grid), 8);
    gtk_grid_set_column_spacing(GTK_GRID(grid), 10);
    gtk_widget_set_margin_start(grid, 12);
    gtk_widget_set_margin_end(grid, 12);
    gtk_widget_set_margin_top(grid, 12);
    gtk_widget_set_margin_bottom(grid, 12);
    gtk_container_add(GTK_CONTAINER(gtk_dialog_get_content_area(GTK_DIALOG(dialog))), grid);
    *grid_out = grid;
    return dialog;
}

static void add_process_clicked(GtkButton *button, gpointer data) {
    GuiState *app = data;
    (void)button;
    if (process_count(app->system) >= DEADLOCK_MAX_PROCESSES) {
        show_message(app, GTK_MESSAGE_WARNING, "The maximum number of processes has been reached.");
        return;
    }
    GtkWidget *grid, *entry;
    GtkWidget *dialog = create_form_dialog(app, "Create Process", "Add Process", &grid);
    gtk_grid_attach(GTK_GRID(grid), gtk_label_new("Process ID (PID):"), 0, 0, 1, 1);
    entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(entry), "Enter a unique integer PID");
    gtk_grid_attach(GTK_GRID(grid), entry, 1, 0, 1, 1);
    gtk_widget_show_all(dialog);
    int response = gtk_dialog_run(GTK_DIALOG(dialog));
    const char *error = NULL;
    if (response == GTK_RESPONSE_ACCEPT) {
        int pid, count = process_count(app->system), i;
        if (!parse_integer(gtk_entry_get_text(GTK_ENTRY(entry)), &pid)) {
            error = "Enter a valid integer PID.";
        } else {
            for (i = 0; i < count && app->system->pid[i] != pid; i++) { }
            if (i != count) error = "That PID already exists. Enter a unique PID.";
            else if (!deadlock_add_process(app->system, pid))
                error = "The process could not be added. Check the system state and process limit.";
        }
    }
    gtk_widget_destroy(dialog);
    if (error) show_message(app, GTK_MESSAGE_WARNING, error);
    else if (response == GTK_RESPONSE_ACCEPT) refresh_all(app);
}

static void add_resource_clicked(GtkButton *button, gpointer data) {
    GuiState *app = data;
    (void)button;
    if (resource_count(app->system) >= DEADLOCK_MAX_RESOURCES) {
        show_message(app, GTK_MESSAGE_WARNING, "The maximum number of resource types has been reached.");
        return;
    }
    GtkWidget *grid;
    GtkWidget *dialog = create_form_dialog(app, "Create Resource Type", "Add Resource", &grid);
    gtk_grid_attach(GTK_GRID(grid), gtk_label_new("Total instances:"), 0, 0, 1, 1);
    GtkWidget *spin = gtk_spin_button_new_with_range(0, 100000, 1);
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(spin), 1);
    gtk_grid_attach(GTK_GRID(grid), spin, 1, 0, 1, 1);
    gtk_widget_show_all(dialog);
    int response = gtk_dialog_run(GTK_DIALOG(dialog));
    int total = (int)gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(spin));
    gtk_widget_destroy(dialog);
    if (response == GTK_RESPONSE_ACCEPT) {
        if (!deadlock_add_resource(app->system, total))
            show_message(app, GTK_MESSAGE_WARNING, "The resource type could not be added. Check the instance count and resource limit.");
        else refresh_all(app);
    }
}

static void edit_matrix_cell(GuiState *app, MatrixCell cell) {
    if (!process_count(app->system) || !resource_count(app->system)) {
        show_message(app, GTK_MESSAGE_WARNING, "Create at least one process and one resource type first.");
        return;
    }
    const char *title = cell == CELL_ALLOCATION ? "Set Allocation Matrix Value" :
        cell == CELL_MAXIMUM ? "Set Maximum Matrix Value" : "Set Request Matrix Value";
    GtkWidget *grid;
    GtkWidget *dialog = create_form_dialog(app, title, "Save Value", &grid);
    GtkWidget *process = create_index_combo(app->system, TRUE);
    GtkWidget *resource = create_index_combo(app->system, FALSE);
    GtkWidget *spin = gtk_spin_button_new_with_range(0, 100000, 1);
    gtk_grid_attach(GTK_GRID(grid), gtk_label_new("Process:"), 0, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), process, 1, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), gtk_label_new("Resource:"), 0, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), resource, 1, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), gtk_label_new("Instances:"), 0, 2, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), spin, 1, 2, 1, 1);
    gtk_widget_show_all(dialog);
    int response = gtk_dialog_run(GTK_DIALOG(dialog)), p, r, ok = 0;
    if (response == GTK_RESPONSE_ACCEPT && combo_active_index(process, &p) &&
        combo_active_index(resource, &r)) {
        int value = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(spin));
        if (cell == CELL_ALLOCATION) ok = deadlock_set_allocation(app->system, p, r, value);
        else if (cell == CELL_MAXIMUM) ok = deadlock_set_maximum(app->system, p, r, value);
        else ok = deadlock_set_request(app->system, p, r, value);
    }
    gtk_widget_destroy(dialog);
    if (response == GTK_RESPONSE_ACCEPT) {
        if (!ok) show_message(app, GTK_MESSAGE_WARNING,
            "The value could not be saved. Check the process, resource, allocation, and maximum requirements.");
        else refresh_all(app);
    }
}

static void set_allocation_clicked(GtkButton *b, gpointer data) {
    (void)b;
    edit_matrix_cell(data, CELL_ALLOCATION);
}
static void set_maximum_clicked(GtkButton *b, gpointer data) {
    (void)b;
    edit_matrix_cell(data, CELL_MAXIMUM);
}
static void set_request_clicked(GtkButton *b, gpointer data) {
    (void)b;
    edit_matrix_cell(data, CELL_REQUEST);
}

static void submit_resource_request_clicked(GtkButton *button, gpointer data) {
    GuiState *app = data;
    (void)button;
    int resources = resource_count(app->system);
    if (!process_count(app->system) || !resources) {
        show_message(app, GTK_MESSAGE_WARNING, "Create at least one process and one resource type first.");
        return;
    }
    GtkWidget *grid;
    GtkWidget *dialog = create_form_dialog(app, "Request Resources", "Submit Request", &grid);
    GtkWidget *process = create_index_combo(app->system, TRUE);
    GtkWidget *spins[DEADLOCK_MAX_RESOURCES];
    gtk_grid_attach(GTK_GRID(grid), gtk_label_new("Process:"), 0, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), process, 1, 0, 1, 1);
    for (int r = 0; r < resources; r++) {
        char *label = g_strdup_printf("Request for R%d:", r + 1);
        gtk_grid_attach(GTK_GRID(grid), gtk_label_new(label), 0, r + 1, 1, 1);
        g_free(label);
        spins[r] = gtk_spin_button_new_with_range(0, 100000, 1);
        gtk_grid_attach(GTK_GRID(grid), spins[r], 1, r + 1, 1, 1);
    }
    gtk_widget_show_all(dialog);
    int response = gtk_dialog_run(GTK_DIALOG(dialog)), p;
    if (response == GTK_RESPONSE_ACCEPT && combo_active_index(process, &p)) {
        int request[DEADLOCK_MAX_RESOURCES] = {0};
        for (int r = 0; r < resources; r++)
            request[r] = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(spins[r]));
        BankerResult result = {0};
        DeadlockRequestStatus status = request_resources(app->system, p, request, &result);
        GString *out = g_string_new("Resource Request Result\n\n");
        g_string_append_printf(out, "%s\n", request_status_text(status));
        if (status == DEADLOCK_REQUEST_GRANTED) append_safe_sequence(out, &result);
        refresh_results(app, out->str);
        g_string_free(out, TRUE);
    }
    gtk_widget_destroy(dialog);
    if (response == GTK_RESPONSE_ACCEPT) refresh_all(app);
}

static void run_bankers_clicked(GtkButton *button, gpointer data) {
    GuiState *app = data;
    (void)button;
    BankerResult result = {0};
    DeadlockSafetyStatus status = run_bankers_algorithm(app->system, &result);
    GString *out = g_string_new("Banker's Algorithm\n\n");
    g_string_append_printf(out, "Safety status: %s\n", safety_status_text(status));
    if (status == DEADLOCK_SAFETY_SAFE) append_safe_sequence(out, &result);
    else g_string_append(out, "No safe sequence was returned for the current state.\n");
    refresh_results(app, out->str);
    g_string_free(out, TRUE);
    refresh_all(app);
}

static void detect_deadlock_clicked(GtkButton *button, gpointer data) {
    GuiState *app = data;
    (void)button;
    DeadlockDetectionResult result = {0};
    DeadlockDetectionStatus status = detect_deadlock(app->system, &result);
    GString *out = g_string_new("Deadlock Detection\n\n");
    g_string_append_printf(out, "Detection result: %s\n", detection_status_text(status));
    if (status == DEADLOCK_DETECTION_FOUND || status == DEADLOCK_DETECTION_NO_DEADLOCK)
        append_deadlocked_processes(out, &result);
    refresh_results(app, out->str);
    g_string_free(out, TRUE);
    refresh_all(app);
}

static void recover_from_deadlock_clicked(GtkButton *button, gpointer data) {
    GuiState *app = data;
    (void)button;
    if (!process_count(app->system)) {
        show_message(app, GTK_MESSAGE_WARNING, "Create at least one process first.");
        return;
    }
    GtkWidget *grid;
    GtkWidget *dialog = create_form_dialog(app, "Recover from Deadlock", "Recover", &grid);
    GtkWidget *process = create_index_combo(app->system, TRUE);
    GtkWidget *strategy = gtk_combo_box_text_new();
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(strategy), "0", "Preempt and release allocated resources");
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(strategy), "1", "Terminate process and release resources");
    gtk_combo_box_set_active(GTK_COMBO_BOX(strategy), 0);
    gtk_grid_attach(GTK_GRID(grid), gtk_label_new("Process:"), 0, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), process, 1, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), gtk_label_new("Recovery strategy:"), 0, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), strategy, 1, 1, 1, 1);
    gtk_widget_show_all(dialog);
    int response = gtk_dialog_run(GTK_DIALOG(dialog)), p, choice;
    if (response == GTK_RESPONSE_ACCEPT && combo_active_index(process, &p) &&
        combo_active_index(strategy, &choice)) {
        DeadlockRecoveryStrategy method = choice == 0
            ? DEADLOCK_RECOVERY_PREEMPT_RESOURCES : DEADLOCK_RECOVERY_TERMINATE_PROCESS;
        int ok = recover_from_deadlock(app->system, p, method);
        GString *out = g_string_new("Deadlock Recovery\n\n");
        g_string_append_printf(out, "%s\n", ok ? "Recovery operation completed." : "Recovery operation failed.");
        refresh_results(app, out->str);
        g_string_free(out, TRUE);
    }
    gtk_widget_destroy(dialog);
    if (response == GTK_RESPONSE_ACCEPT) refresh_all(app);
}

static void system_status_clicked(GtkButton *button, gpointer data) {
    GuiState *app = data;
    (void)button;
    BankerResult banker = {0};
    DeadlockDetectionResult detection = {0};
    DeadlockSystemStatus status = evaluate_system_status(app->system, &banker, &detection);
    GString *out = g_string_new("System Status\n\n");
    g_string_append_printf(out, "Current state: %s\n", system_status_text(status));
    if (status == DEADLOCK_SYSTEM_STATUS_SAFE) append_safe_sequence(out, &banker);
    if (status == DEADLOCK_SYSTEM_STATUS_DEADLOCKED || status == DEADLOCK_SYSTEM_STATUS_UNSAFE)
        append_deadlocked_processes(out, &detection);
    refresh_results(app, out->str);
    g_string_free(out, TRUE);
    refresh_all(app);
}

static void refresh_clicked(GtkButton *button, gpointer data) {
    (void)button;
    refresh_all(data);
}

static void draw_centered_text(cairo_t *cr, const char *text, double cx, double cy) {
    cairo_text_extents_t extents;
    cairo_text_extents(cr, text, &extents);
    cairo_move_to(cr, cx - extents.width / 2.0 - extents.x_bearing,
                  cy - extents.height / 2.0 - extents.y_bearing);
    cairo_show_text(cr, text);
}

static void graph_node_position(const DeadlockSystem *s, RAGNodeType type, int index,
                                double width, double height, double *x, double *y) {
    int count = type == RAG_NODE_PROCESS ? process_count(s) : resource_count(s);
    double usable = height - 58.0 - 35.0;
    if (count < 1) count = 1;
    if (usable < 1.0) usable = 1.0;
    *x = type == RAG_NODE_PROCESS ? width * 0.25 : width * 0.75;
    *y = 58.0 + usable * (index + 1.0) / (count + 1.0);
}

static void draw_arrow(cairo_t *cr, double x1, double y1, double x2, double y2,
                       gboolean request, int instances) {
    double dx = x2 - x1, dy = y2 - y1, length = sqrt(dx * dx + dy * dy);
    if (length < 1.0) return;
    double ux = dx / length, uy = dy / length;
    double sx = x1 + ux * 26.0, sy = y1 + uy * 26.0;
    double ex = x2 - ux * 30.0, ey = y2 - uy * 30.0;
    cairo_set_source_rgb(cr, request ? 0.78 : 0.12, request ? 0.20 : 0.36, request ? 0.20 : 0.70);
    cairo_set_line_width(cr, 2.2);
    cairo_move_to(cr, sx, sy);
    cairo_line_to(cr, ex, ey);
    cairo_stroke(cr);
    double angle = atan2(ey - sy, ex - sx), size = 10.0, spread = 0.48;
    cairo_move_to(cr, ex, ey);
    cairo_line_to(cr, ex - size * cos(angle - spread), ey - size * sin(angle - spread));
    cairo_line_to(cr, ex - size * cos(angle + spread), ey - size * sin(angle + spread));
    cairo_close_path(cr);
    cairo_fill(cr);
    if (instances > 0) {
        char label[32];
        g_snprintf(label, sizeof(label), "x%d", instances);
        cairo_set_source_rgb(cr, 0.12, 0.12, 0.12);
        cairo_select_font_face(cr, "Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
        cairo_set_font_size(cr, 12.0);
        draw_centered_text(cr, label, (sx + ex) / 2.0, (sy + ey) / 2.0 - 6.0);
    }
}

static gboolean draw_resource_graph(GtkWidget *widget, cairo_t *cr, gpointer data) {
    GuiState *app = data;
    DeadlockSystem *s = app->system;
    GtkAllocation area;
    int processes = process_count(s), resources = resource_count(s);
    gtk_widget_get_allocation(widget, &area);
    double width = area.width > 0 ? area.width : 950.0;
    double height = area.height > 0 ? area.height : 520.0;
    cairo_set_source_rgb(cr, 1, 1, 1);
    cairo_paint(cr);
    cairo_select_font_face(cr, "Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
    cairo_set_font_size(cr, 15.0);
    cairo_set_source_rgb(cr, 0.20, 0.20, 0.20);
    cairo_move_to(cr, 35, 28);
    cairo_show_text(cr, "Processes");
    cairo_move_to(cr, width * 0.70, 28);
    cairo_show_text(cr, "Resource Types");
    if (!processes || !resources) {
        cairo_set_font_size(cr, 17.0);
        cairo_move_to(cr, 35, 75);
        cairo_show_text(cr, "Create processes and resource types to display the Resource Allocation Graph.");
        return FALSE;
    }

    ResourceAllocationGraph graph = {0};
    build_resource_allocation_graph(s, &graph);
    int edges = CLAMP(graph.edge_count, 0, DEADLOCK_MAX_GRAPH_EDGES);
    for (int i = 0; i < edges; i++) {
        ResourceGraphEdge *e = &graph.edges[i];
        int from_limit = e->from_type == RAG_NODE_PROCESS ? processes : resources;
        int to_limit = e->to_type == RAG_NODE_PROCESS ? processes : resources;
        if (e->from_index < 0 || e->from_index >= from_limit ||
            e->to_index < 0 || e->to_index >= to_limit) continue;
        double x1, y1, x2, y2;
        graph_node_position(s, e->from_type, e->from_index, width, height, &x1, &y1);
        graph_node_position(s, e->to_type, e->to_index, width, height, &x2, &y2);
        draw_arrow(cr, x1, y1, x2, y2, e->edge_type == RAG_EDGE_REQUEST, e->instances);
    }

    for (int i = 0; i < processes; i++) {
        double x, y;
        char label[40];
        graph_node_position(s, RAG_NODE_PROCESS, i, width, height, &x, &y);
        if (s->active[i]) cairo_set_source_rgb(cr, 0.79, 0.88, 0.98);
        else cairo_set_source_rgb(cr, 0.84, 0.84, 0.84);
        cairo_arc(cr, x, y, 25, 0, 2 * G_PI);
        cairo_fill_preserve(cr);
        cairo_set_source_rgb(cr, 0.17, 0.31, 0.47);
        cairo_set_line_width(cr, 1.5);
        cairo_stroke(cr);
        cairo_set_source_rgb(cr, 0.08, 0.12, 0.18);
        cairo_set_font_size(cr, 13);
        g_snprintf(label, sizeof(label), "P%d", s->pid[i]);
        draw_centered_text(cr, label, x, y);
    }
    for (int i = 0; i < resources; i++) {
        double x, y;
        char label[40], count_label[40];
        graph_node_position(s, RAG_NODE_RESOURCE, i, width, height, &x, &y);
        cairo_rectangle(cr, x - 39, y - 22, 78, 44);
        cairo_set_source_rgb(cr, 0.99, 0.88, 0.68);
        cairo_fill_preserve(cr);
        cairo_set_source_rgb(cr, 0.47, 0.33, 0.15);
        cairo_set_line_width(cr, 1.5);
        cairo_stroke(cr);
        cairo_set_source_rgb(cr, 0.15, 0.12, 0.08);
        cairo_set_font_size(cr, 13);
        g_snprintf(label, sizeof(label), "R%d", i + 1);
        draw_centered_text(cr, label, x, y - 4);
        cairo_set_font_size(cr, 10);
        g_snprintf(count_label, sizeof(count_label), "Total: %d", s->total[i]);
        draw_centered_text(cr, count_label, x, y + 12);
    }
    cairo_set_font_size(cr, 12);
    cairo_set_source_rgb(cr, 0.12, 0.36, 0.70);
    cairo_move_to(cr, 35, height - 15);
    cairo_show_text(cr, "Blue: allocation edge");
    cairo_set_source_rgb(cr, 0.78, 0.20, 0.20);
    cairo_move_to(cr, 220, height - 15);
    cairo_show_text(cr, "Red: request edge");
    return FALSE;
}

static void add_button(GtkGrid *grid, const char *label, int column, int row,
                       GCallback callback, GuiState *app) {
    GtkWidget *button = gtk_button_new_with_label(label);
    gtk_widget_set_hexpand(button, TRUE);
    gtk_widget_set_size_request(button, 150, 36);
    g_signal_connect(button, "clicked", callback, app);
    gtk_grid_attach(grid, button, column, row, 1, 1);
}

static GtkWidget *create_scrolled_page(GtkWidget *child) {
    GtkWidget *scroll = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll),
                                   GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_container_add(GTK_CONTAINER(scroll), child);
    return scroll;
}

static void add_output_tab(GtkWidget *notebook, GtkWidget **view, const char *title) {
    *view = create_output_view();
    gtk_notebook_append_page(GTK_NOTEBOOK(notebook), create_scrolled_page(*view), gtk_label_new(title));
}

void gui_run(DeadlockSystem *system) {
    GuiState app = {0};
    static DeadlockSystem fallback_system;
    if (!system) {
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

    GtkWidget *root = gtk_box_new(GTK_ORIENTATION_VERTICAL, 8);
    gtk_widget_set_margin_start(root, 12);
    gtk_widget_set_margin_end(root, 12);
    gtk_widget_set_margin_top(root, 10);
    gtk_widget_set_margin_bottom(root, 10);
    gtk_container_add(GTK_CONTAINER(app.window), root);
    GtkWidget *title = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(title), "<span size='xx-large' weight='bold'>Deadlock Simulation System</span>");
    gtk_widget_set_halign(title, GTK_ALIGN_START);
    gtk_box_pack_start(GTK_BOX(root), title, FALSE, FALSE, 2);

    GtkWidget *frame = gtk_frame_new("System Operations");
    gtk_box_pack_start(GTK_BOX(root), frame, FALSE, FALSE, 2);
    GtkWidget *grid = gtk_grid_new();
    gtk_grid_set_row_spacing(GTK_GRID(grid), 6);
    gtk_grid_set_column_spacing(GTK_GRID(grid), 6);
    gtk_widget_set_margin_start(grid, 8);
    gtk_widget_set_margin_end(grid, 8);
    gtk_widget_set_margin_top(grid, 8);
    gtk_widget_set_margin_bottom(grid, 8);
    gtk_container_add(GTK_CONTAINER(frame), grid);
    add_button(GTK_GRID(grid), "Create Process", 0, 0, G_CALLBACK(add_process_clicked), &app);
    add_button(GTK_GRID(grid), "Create Resource", 1, 0, G_CALLBACK(add_resource_clicked), &app);
    add_button(GTK_GRID(grid), "Set Allocation", 2, 0, G_CALLBACK(set_allocation_clicked), &app);
    add_button(GTK_GRID(grid), "Set Maximum", 0, 1, G_CALLBACK(set_maximum_clicked), &app);
    add_button(GTK_GRID(grid), "Set Request", 1, 1, G_CALLBACK(set_request_clicked), &app);
    add_button(GTK_GRID(grid), "Submit Resource Request", 2, 1, G_CALLBACK(submit_resource_request_clicked), &app);
    add_button(GTK_GRID(grid), "Run Banker's Algorithm", 0, 2, G_CALLBACK(run_bankers_clicked), &app);
    add_button(GTK_GRID(grid), "Detect Deadlock", 1, 2, G_CALLBACK(detect_deadlock_clicked), &app);
    add_button(GTK_GRID(grid), "Recover from Deadlock", 2, 2, G_CALLBACK(recover_from_deadlock_clicked), &app);
    add_button(GTK_GRID(grid), "System Status", 0, 3, G_CALLBACK(system_status_clicked), &app);
    add_button(GTK_GRID(grid), "Refresh Display", 1, 3, G_CALLBACK(refresh_clicked), &app);

    GtkWidget *notebook = gtk_notebook_new();
    gtk_widget_set_vexpand(notebook, TRUE);
    gtk_box_pack_start(GTK_BOX(root), notebook, TRUE, TRUE, 2);
    add_output_tab(notebook, &app.matrix_view, "System Matrices");
    add_output_tab(notebook, &app.results_view, "Algorithm Results");

    app.graph_area = gtk_drawing_area_new();
    gtk_widget_set_size_request(app.graph_area, 950, 520);
    g_signal_connect(app.graph_area, "draw", G_CALLBACK(draw_resource_graph), &app);
    GtkWidget *graph_page = gtk_box_new(GTK_ORIENTATION_VERTICAL, 4);
    GtkWidget *legend = gtk_label_new("Process nodes are circles. Resource types are rectangles. Blue edges indicate allocations; red edges indicate requests.");
    gtk_label_set_xalign(GTK_LABEL(legend), 0.0f);
    gtk_box_pack_start(GTK_BOX(graph_page), legend, FALSE, FALSE, 5);
    gtk_box_pack_start(GTK_BOX(graph_page), create_scrolled_page(app.graph_area), TRUE, TRUE, 0);
    gtk_notebook_append_page(GTK_NOTEBOOK(notebook), graph_page, gtk_label_new("Resource Allocation Graph"));
    add_output_tab(notebook, &app.utilization_view, "Resource Utilization");

    GtkWidget *status_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);
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
