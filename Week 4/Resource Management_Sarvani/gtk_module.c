#include "gtk_module.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

static void status_set(Student2GtkState *s, const char *message) {
    if (s->status_label)
        gtk_label_set_text(s->status_label, message);
}

static void message_show(Student2GtkState *s, GtkMessageType type,
                         const char *title, const char *message) {
    GtkWidget *dialog = gtk_message_dialog_new(
        s->parent_window ? GTK_WINDOW(s->parent_window) : NULL,
        GTK_DIALOG_MODAL, type, GTK_BUTTONS_OK, "%s", message);
    gtk_window_set_title(GTK_WINDOW(dialog), title);
    gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);
}

static void addf(char *buf, size_t cap, size_t *used,
                 const char *fmt, ...) {
    if (*used >= cap) return;
    va_list ap;
    va_start(ap, fmt);
    int n = vsnprintf(buf + *used, cap - *used, fmt, ap);
    va_end(ap);
    if (n > 0) {
        size_t remaining = cap - *used;
        *used += ((size_t)n >= remaining) ? remaining - 1 : (size_t)n;
    }
}

static void recalc_available(Student2GtkState *s) {
    for (int r = 0; r < s->resource_count; r++) {
        int used = 0;
        for (int p = 0; p < s->process_count; p++)
            used += s->allocation[p][r];
        s->available[r] = s->total[r] - used;
    }
}

static void recalc_need(Student2GtkState *s) {
    for (int p = 0; p < s->process_count; p++)
        for (int r = 0; r < s->resource_count; r++)
            s->need[p][r] = s->maximum[p][r] - s->allocation[p][r];
}

void student2_gtk_init(Student2GtkState *s, GtkWidget *parent_window,
                       GtkTextBuffer *matrix_buffer, GtkLabel *status_label) {
    memset(s, 0, sizeof(*s));
    s->parent_window = parent_window;
    s->matrix_buffer = matrix_buffer;
    s->status_label = status_label;
    status_set(s, "Add processes and resources to begin.");
    student2_gtk_refresh_matrices(s);
}

void student2_gtk_refresh_matrices(Student2GtkState *s) {
    if (!s || !s->matrix_buffer) return;
    recalc_need(s);

    char buf[30000] = "";
    size_t used = 0;
    addf(buf, sizeof(buf), &used,
         "Processes: %d    Resource types: %d\n\nAvailable: [",
         s->process_count, s->resource_count);
    for (int r = 0; r < s->resource_count; r++)
        addf(buf, sizeof(buf), &used, "%s%d", r ? ", " : "", s->available[r]);
    addf(buf, sizeof(buf), &used, "]\nTotal: [");
    for (int r = 0; r < s->resource_count; r++)
        addf(buf, sizeof(buf), &used, "%s%d", r ? ", " : "", s->total[r]);
    addf(buf, sizeof(buf), &used, "]\n");

    if (s->process_count == 0)
        addf(buf, sizeof(buf), &used, "\nAdd at least one process to begin.\n");
    if (s->resource_count == 0)
        addf(buf, sizeof(buf), &used, "\nAdd at least one resource type to begin.\n");

    const char *names[] = {"Allocation Matrix", "Maximum Matrix",
                           "Need Matrix", "Request Matrix"};
    for (int m = 0; m < 4; m++) {
        addf(buf, sizeof(buf), &used, "\n%s\nPID\t", names[m]);
        for (int r = 0; r < s->resource_count; r++)
            addf(buf, sizeof(buf), &used, "%s\t", s->resource_names[r]);
        addf(buf, sizeof(buf), &used, "\n");

        for (int p = 0; p < s->process_count; p++) {
            addf(buf, sizeof(buf), &used, "%s\t", s->process_names[p]);
            for (int r = 0; r < s->resource_count; r++) {
                int value = 0;
                if (m == 0) value = s->allocation[p][r];
                else if (m == 1) value = s->maximum[p][r];
                else if (m == 2) value = s->need[p][r];
                else value = s->request[p][r];
                addf(buf, sizeof(buf), &used, "%d\t", value);
            }
            addf(buf, sizeof(buf), &used, "\n");
        }
    }

    GtkTextIter start, end;
    gtk_text_buffer_get_bounds(s->matrix_buffer, &start, &end);
    gtk_text_buffer_delete(s->matrix_buffer, &start, &end);
    gtk_text_buffer_get_start_iter(s->matrix_buffer, &start);
    gtk_text_buffer_insert(s->matrix_buffer, &start, buf, -1);
}

static gboolean ask_text(Student2GtkState *s, const char *title,
                         const char *prompt, char *out, size_t out_size) {
    GtkWidget *dialog = gtk_dialog_new_with_buttons(
        title, s->parent_window ? GTK_WINDOW(s->parent_window) : NULL,
        GTK_DIALOG_MODAL, "_Cancel", GTK_RESPONSE_CANCEL,
        "_OK", GTK_RESPONSE_OK, NULL);
    GtkWidget *content = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 8);
    gtk_container_set_border_width(GTK_CONTAINER(box), 12);
    GtkWidget *label = gtk_label_new(prompt);
    GtkWidget *entry = gtk_entry_new();
    gtk_box_pack_start(GTK_BOX(box), label, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(box), entry, FALSE, FALSE, 0);
    gtk_container_add(GTK_CONTAINER(content), box);
    gtk_widget_show_all(dialog);

    gboolean ok = gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_OK;
    if (ok) {
        const char *v = gtk_entry_get_text(GTK_ENTRY(entry));
        if (!v[0] || strlen(v) >= out_size) {
            message_show(s, GTK_MESSAGE_WARNING, "Invalid input",
                         "Enter a non-empty value within the allowed length.");
            ok = FALSE;
        } else {
            g_strlcpy(out, v, out_size);
        }
    }
    gtk_widget_destroy(dialog);
    return ok;
}

static gboolean ask_nonnegative_int(Student2GtkState *s, const char *title,
                                    const char *prompt, int *out) {
    char text[64];
    if (!ask_text(s, title, prompt, text, sizeof(text))) return FALSE;
    char *end = NULL;
    long v = strtol(text, &end, 10);
    if (!text[0] || !end || *end || v < 0 || v > 1000000) {
        message_show(s, GTK_MESSAGE_WARNING, "Invalid number",
                     "Enter a whole number from 0 to 1000000.");
        return FALSE;
    }
    *out = (int)v;
    return TRUE;
}

static int choose_process(Student2GtkState *s, const char *title) {
    if (!s->process_count) {
        message_show(s, GTK_MESSAGE_INFO, title, "Create a process first.");
        return -1;
    }
    GtkWidget *d = gtk_dialog_new_with_buttons(
        title, s->parent_window ? GTK_WINDOW(s->parent_window) : NULL,
        GTK_DIALOG_MODAL, "_Cancel", GTK_RESPONSE_CANCEL,
        "_OK", GTK_RESPONSE_OK, NULL);
    GtkWidget *combo = gtk_combo_box_text_new();
    for (int p = 0; p < s->process_count; p++)
        gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(combo), s->process_names[p]);
    gtk_combo_box_set_active(GTK_COMBO_BOX(combo), 0);
    GtkWidget *content = gtk_dialog_get_content_area(GTK_DIALOG(d));
    gtk_box_pack_start(GTK_BOX(content), gtk_label_new("Select process:"), FALSE, FALSE, 6);
    gtk_box_pack_start(GTK_BOX(content), combo, FALSE, FALSE, 6);
    gtk_widget_show_all(d);
    int result = gtk_dialog_run(GTK_DIALOG(d)) == GTK_RESPONSE_OK
        ? gtk_combo_box_get_active(GTK_COMBO_BOX(combo)) : -1;
    gtk_widget_destroy(d);
    return result;
}

static int choose_resource(Student2GtkState *s, const char *title) {
    if (!s->resource_count) {
        message_show(s, GTK_MESSAGE_INFO, title, "Create a resource first.");
        return -1;
    }
    GtkWidget *d = gtk_dialog_new_with_buttons(
        title, s->parent_window ? GTK_WINDOW(s->parent_window) : NULL,
        GTK_DIALOG_MODAL, "_Cancel", GTK_RESPONSE_CANCEL,
        "_OK", GTK_RESPONSE_OK, NULL);
    GtkWidget *combo = gtk_combo_box_text_new();
    for (int r = 0; r < s->resource_count; r++)
        gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(combo), s->resource_names[r]);
    gtk_combo_box_set_active(GTK_COMBO_BOX(combo), 0);
    GtkWidget *content = gtk_dialog_get_content_area(GTK_DIALOG(d));
    gtk_box_pack_start(GTK_BOX(content), gtk_label_new("Select resource type:"), FALSE, FALSE, 6);
    gtk_box_pack_start(GTK_BOX(content), combo, FALSE, FALSE, 6);
    gtk_widget_show_all(d);
    int result = gtk_dialog_run(GTK_DIALOG(d)) == GTK_RESPONSE_OK
        ? gtk_combo_box_get_active(GTK_COMBO_BOX(combo)) : -1;
    gtk_widget_destroy(d);
    return result;
}

void student2_gtk_create_process(GtkWidget *button, gpointer data) {
    (void)button;
    Student2GtkState *s = data;
    if (s->process_count >= S2_MAX_PROCESSES) {
        message_show(s, GTK_MESSAGE_WARNING, "Create Process", "Maximum process count reached.");
        return;
    }
    char name[S2_NAME_LEN];
    if (!ask_text(s, "Create Process", "Enter a process name / PID:", name, sizeof(name))) return;
    for (int p = 0; p < s->process_count; p++) {
        if (!strcmp(s->process_names[p], name)) {
            message_show(s, GTK_MESSAGE_WARNING, "Create Process", "That process name already exists.");
            return;
        }
    }
    g_strlcpy(s->process_names[s->process_count++], name, S2_NAME_LEN);
    student2_gtk_refresh_matrices(s);
    status_set(s, "Process created.");
}

void student2_gtk_create_resource(GtkWidget *button, gpointer data) {
    (void)button;
    Student2GtkState *s = data;
    if (s->resource_count >= S2_MAX_RESOURCES) {
        message_show(s, GTK_MESSAGE_WARNING, "Create Resource", "Maximum resource types reached.");
        return;
    }
    char name[S2_NAME_LEN];
    int total;
    if (!ask_text(s, "Create Resource", "Enter resource type name:", name, sizeof(name))) return;
    for (int r = 0; r < s->resource_count; r++) {
        if (!strcmp(s->resource_names[r], name)) {
            message_show(s, GTK_MESSAGE_WARNING, "Create Resource", "That resource name already exists.");
            return;
        }
    }
    if (!ask_nonnegative_int(s, "Create Resource", "Enter total instances:", &total)) return;
    int r = s->resource_count++;
    g_strlcpy(s->resource_names[r], name, S2_NAME_LEN);
    s->total[r] = total;
    s->available[r] = total;
    student2_gtk_refresh_matrices(s);
    status_set(s, "Resource type created.");
}

void student2_gtk_set_maximum(GtkWidget *button, gpointer data) {
    (void)button;
    Student2GtkState *s = data;
    int p = choose_process(s, "Set Maximum");
    if (p < 0) return;
    int r = choose_resource(s, "Set Maximum");
    if (r < 0) return;
    int value;
    if (!ask_nonnegative_int(s, "Set Maximum", "Enter maximum required instances:", &value)) return;
    if (value < s->allocation[p][r] || value > s->total[r]) {
        message_show(s, GTK_MESSAGE_WARNING, "Set Maximum",
                     "Maximum must be at least the allocation and no greater than total instances.");
        return;
    }
    s->maximum[p][r] = value;
    recalc_need(s);
    student2_gtk_refresh_matrices(s);
    status_set(s, "Maximum requirement updated.");
}

void student2_gtk_set_allocation(GtkWidget *button, gpointer data) {
    (void)button;
    Student2GtkState *s = data;
    int p = choose_process(s, "Set Allocation");
    if (p < 0) return;
    int r = choose_resource(s, "Set Allocation");
    if (r < 0) return;
    int value;
    if (!ask_nonnegative_int(s, "Set Allocation", "Enter current allocated instances:", &value)) return;
    if (value > s->maximum[p][r]) {
        message_show(s, GTK_MESSAGE_WARNING, "Set Allocation",
                     "Set Maximum first. Allocation cannot exceed maximum.");
        return;
    }
    int old = s->allocation[p][r];
    s->allocation[p][r] = value;
    recalc_available(s);
    if (s->available[r] < 0) {
        s->allocation[p][r] = old;
        recalc_available(s);
        message_show(s, GTK_MESSAGE_WARNING, "Set Allocation",
                     "Total allocations cannot exceed total resource instances.");
        return;
    }
    recalc_need(s);
    student2_gtk_refresh_matrices(s);
    status_set(s, "Allocation updated.");
}

void student2_gtk_set_request(GtkWidget *button, gpointer data) {
    (void)button;
    Student2GtkState *s = data;
    int p = choose_process(s, "Set Request");
    if (p < 0) return;
    if (!s->resource_count) {
        message_show(s, GTK_MESSAGE_INFO, "Set Request", "Create a resource first.");
        return;
    }

    GtkWidget *d = gtk_dialog_new_with_buttons(
        "Set Request", s->parent_window ? GTK_WINDOW(s->parent_window) : NULL,
        GTK_DIALOG_MODAL, "_Cancel", GTK_RESPONSE_CANCEL,
        "_Save Request", GTK_RESPONSE_OK, NULL);
    GtkWidget *content = gtk_dialog_get_content_area(GTK_DIALOG(d));
    GtkWidget *grid = gtk_grid_new();
    gtk_grid_set_row_spacing(GTK_GRID(grid), 8);
    gtk_grid_set_column_spacing(GTK_GRID(grid), 12);
    gtk_container_set_border_width(GTK_CONTAINER(grid), 12);
    GtkWidget *entries[S2_MAX_RESOURCES] = {0};
    for (int r = 0; r < s->resource_count; r++) {
        gtk_grid_attach(GTK_GRID(grid), gtk_label_new(s->resource_names[r]), 0, r, 1, 1);
        entries[r] = gtk_entry_new();
        char initial[32];
        snprintf(initial, sizeof(initial), "%d", s->request[p][r]);
        gtk_entry_set_text(GTK_ENTRY(entries[r]), initial);
        gtk_grid_attach(GTK_GRID(grid), entries[r], 1, r, 1, 1);
    }
    gtk_container_add(GTK_CONTAINER(content), grid);
    gtk_widget_show_all(d);
    if (gtk_dialog_run(GTK_DIALOG(d)) == GTK_RESPONSE_OK) {
        int values[S2_MAX_RESOURCES] = {0};
        gboolean valid = TRUE;
        for (int r = 0; r < s->resource_count; r++) {
            const char *txt = gtk_entry_get_text(GTK_ENTRY(entries[r]));
            char *end = NULL;
            long v = strtol(txt, &end, 10);
            if (!txt[0] || !end || *end || v < 0 || v > 1000000) {
                valid = FALSE;
                break;
            }
            values[r] = (int)v;
        }
        if (valid) {
            for (int r = 0; r < s->resource_count; r++)
                s->request[p][r] = values[r];
            student2_gtk_refresh_matrices(s);
            status_set(s, "Request saved. Click Submit Resource Request.");
        } else {
            message_show(s, GTK_MESSAGE_WARNING, "Set Request",
                         "Enter a non-negative whole number for each resource.");
        }
    }
    gtk_widget_destroy(d);
}

static int bankers(Student2GtkState *s, int sequence[]) {
    int work[S2_MAX_RESOURCES];
    int finish[S2_MAX_PROCESSES] = {0};
    int count = 0;
    if (!s->process_count || !s->resource_count) return 0;
    recalc_need(s);
    for (int r = 0; r < s->resource_count; r++) work[r] = s->available[r];

    while (count < s->process_count) {
        int progressed = 0;
        for (int p = 0; p < s->process_count; p++) {
            if (finish[p]) continue;
            int can_finish = 1;
            for (int r = 0; r < s->resource_count; r++)
                if (s->need[p][r] > work[r]) { can_finish = 0; break; }
            if (can_finish) {
                for (int r = 0; r < s->resource_count; r++)
                    work[r] += s->allocation[p][r];
                finish[p] = 1;
                sequence[count++] = p;
                progressed = 1;
            }
        }
        if (!progressed) return 0;
    }
    return 1;
}

void student2_gtk_run_bankers(GtkWidget *button, gpointer data) {
    (void)button;
    Student2GtkState *s = data;
    int seq[S2_MAX_PROCESSES];
    if (bankers(s, seq)) {
        char msg[1200] = "System is SAFE.\nSafe sequence: ";
        for (int i = 0; i < s->process_count; i++) {
            if (i) g_strlcat(msg, " -> ", sizeof(msg));
            g_strlcat(msg, s->process_names[seq[i]], sizeof(msg));
        }
        message_show(s, GTK_MESSAGE_INFO, "Banker's Algorithm", msg);
        status_set(s, "SAFE: a safe sequence exists.");
    } else {
        message_show(s, GTK_MESSAGE_WARNING, "Banker's Algorithm",
                     "System is UNSAFE: no safe sequence exists.");
        status_set(s, "UNSAFE: no safe sequence exists.");
    }
}

void student2_gtk_submit_request(GtkWidget *button, gpointer data) {
    (void)button;
    Student2GtkState *s = data;
    int p = choose_process(s, "Submit Resource Request");
    if (p < 0) return;
    if (!s->resource_count) return;

    int seq[S2_MAX_PROCESSES];
    recalc_need(s);
    for (int r = 0; r < s->resource_count; r++) {
        if (s->request[p][r] > s->need[p][r]) {
            message_show(s, GTK_MESSAGE_WARNING, "Request rejected",
                         "Request exceeds the process's remaining Need.");
            status_set(s, "Request rejected: exceeds Need.");
            return;
        }
        if (s->request[p][r] > s->available[r]) {
            message_show(s, GTK_MESSAGE_WARNING, "Request rejected",
                         "Insufficient resources are currently available.");
            status_set(s, "Request rejected: insufficient Available resources.");
            return;
        }
    }

    for (int r = 0; r < s->resource_count; r++) {
        s->available[r] -= s->request[p][r];
        s->allocation[p][r] += s->request[p][r];
    }
    recalc_need(s);

    if (bankers(s, seq)) {
        student2_gtk_refresh_matrices(s);
        message_show(s, GTK_MESSAGE_INFO, "Request granted",
                     "Request granted. The resulting state is SAFE.");
        status_set(s, "Request granted: resulting state is SAFE.");
    } else {
        for (int r = 0; r < s->resource_count; r++) {
            s->available[r] += s->request[p][r];
            s->allocation[p][r] -= s->request[p][r];
        }
        recalc_need(s);
        student2_gtk_refresh_matrices(s);
        message_show(s, GTK_MESSAGE_WARNING, "Request denied",
                     "Request would make the state UNSAFE. Allocation rolled back.");
        status_set(s, "Request denied: resulting state would be UNSAFE.");
    }
}

void student2_gtk_system_status(GtkWidget *button, gpointer data) {
    (void)button;
    Student2GtkState *s = data;
    int seq[S2_MAX_PROCESSES];
    if (!s->process_count || !s->resource_count) {
        message_show(s, GTK_MESSAGE_INFO, "System Status",
                     "Add at least one process and one resource type.");
        return;
    }
    if (bankers(s, seq)) {
        char msg[1200] = "System Status: SAFE\nSafe sequence: ";
        for (int i = 0; i < s->process_count; i++) {
            if (i) g_strlcat(msg, " -> ", sizeof(msg));
            g_strlcat(msg, s->process_names[seq[i]], sizeof(msg));
        }
        message_show(s, GTK_MESSAGE_INFO, "System Status", msg);
        status_set(s, "System status: SAFE.");
    } else {
        message_show(s, GTK_MESSAGE_WARNING, "System Status",
                     "System Status: UNSAFE. No safe sequence exists.");
        status_set(s, "System status: UNSAFE.");
    }
    student2_gtk_refresh_matrices(s);
}
