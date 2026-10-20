
#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX 20

GtkWidget *window, *process_spin, *resource_spin;
GtkWidget *available_grid, *allocation_grid, *request_grid;
GtkWidget *selected_spin, *output_view;

GtkWidget *available_entries[MAX];
GtkWidget *allocation_entries[MAX][MAX];
GtkWidget *request_entries[MAX][MAX];

int n = 2, m = 2;
int available[MAX];
int allocation[MAX][MAX];
int request[MAX][MAX];
int deadlocked[MAX];

GtkWidget *make_label(const char *text)
{
    return gtk_label_new(text);
}

GtkWidget *make_entry(int value)
{
    GtkWidget *entry = gtk_entry_new();
    char text[20];

    snprintf(text, sizeof(text), "%d", value);
    gtk_entry_set_text(GTK_ENTRY(entry), text);
    gtk_entry_set_width_chars(GTK_ENTRY(entry), 5);
    return entry;
}

void write_output(const char *text)
{
    GtkTextBuffer *buffer =
        gtk_text_view_get_buffer(GTK_TEXT_VIEW(output_view));

    gtk_text_buffer_set_text(buffer, text, -1);
}

void append_output(const char *text)
{
    GtkTextBuffer *buffer =
        gtk_text_view_get_buffer(GTK_TEXT_VIEW(output_view));

    GtkTextIter end;
    gtk_text_buffer_get_end_iter(buffer, &end);
    gtk_text_buffer_insert(buffer, &end, text, -1);
    gtk_text_buffer_insert(buffer, &end, "\n", -1);
}

int read_entry(GtkWidget *entry, int *value)
{
    const char *text =
        gtk_entry_get_text(GTK_ENTRY(entry));
    char *end;
    long number;

    if (text == NULL || *text == '\0')
        return 0;

    number = strtol(text, &end, 10);

    if (*end != '\0' || number < 0 || number > 1000000)
        return 0;

    *value = (int)number;
    return 1;
}

int read_matrices(void)
{
    int i, j;

    for (j = 0; j < m; j++) {
        if (!read_entry(available_entries[j], &available[j]))
            return 0;
    }

    for (i = 0; i < n; i++) {
        for (j = 0; j < m; j++) {
            if (!read_entry(allocation_entries[i][j],
                            &allocation[i][j]))
                return 0;

            if (!read_entry(request_entries[i][j],
                            &request[i][j]))
                return 0;
        }
    }

    return 1;
}

void create_matrices(void)
{
    int i, j;

    n = gtk_spin_button_get_value_as_int(
        GTK_SPIN_BUTTON(process_spin));

    m = gtk_spin_button_get_value_as_int(
        GTK_SPIN_BUTTON(resource_spin));

    gtk_spin_button_set_range(GTK_SPIN_BUTTON(selected_spin), 1, n);

    GList *children =
        gtk_container_get_children(GTK_CONTAINER(available_grid));

    for (GList *p = children; p != NULL; p = p->next)
        gtk_widget_destroy(GTK_WIDGET(p->data));

    g_list_free(children);

    children =
        gtk_container_get_children(GTK_CONTAINER(allocation_grid));

    for (GList *p = children; p != NULL; p = p->next)
        gtk_widget_destroy(GTK_WIDGET(p->data));

    g_list_free(children);

    children =
        gtk_container_get_children(GTK_CONTAINER(request_grid));

    for (GList *p = children; p != NULL; p = p->next)
        gtk_widget_destroy(GTK_WIDGET(p->data));

    g_list_free(children);

    gtk_grid_attach(GTK_GRID(available_grid),
                    make_label("Resource"), 0, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(available_grid),
                    make_label("Units"), 0, 1, 1, 1);

    for (j = 0; j < m; j++) {
        char label[20];

        snprintf(label, sizeof(label), "R%d", j + 1);
        gtk_grid_attach(GTK_GRID(available_grid),
                        make_label(label), j + 1, 0, 1, 1);

        available_entries[j] = make_entry(0);
        gtk_grid_attach(GTK_GRID(available_grid),
                        available_entries[j], j + 1, 1, 1, 1);
    }

    for (int which = 0; which < 2; which++) {
        GtkWidget *grid = which == 0
            ? allocation_grid : request_grid;

        gtk_grid_attach(GTK_GRID(grid),
                        make_label("Process"), 0, 0, 1, 1);

        for (j = 0; j < m; j++) {
            char label[20];

            snprintf(label, sizeof(label), "R%d", j + 1);
            gtk_grid_attach(GTK_GRID(grid),
                            make_label(label), j + 1, 0, 1, 1);
        }

        for (i = 0; i < n; i++) {
            char label[20];

            snprintf(label, sizeof(label), "P%d", i + 1);
            gtk_grid_attach(GTK_GRID(grid),
                            make_label(label), 0, i + 1, 1, 1);

            for (j = 0; j < m; j++) {
                GtkWidget *entry = make_entry(0);

                if (which == 0)
                    allocation_entries[i][j] = entry;
                else
                    request_entries[i][j] = entry;

                gtk_grid_attach(GTK_GRID(grid), entry,
                                j + 1, i + 1, 1, 1);
            }
        }
    }

    gtk_widget_show_all(available_grid);
    gtk_widget_show_all(allocation_grid);
    gtk_widget_show_all(request_grid);

    write_output("Matrices created. Enter resource values to begin.");
}

void detect_deadlock(void)
{
    int work[MAX];
    int finish[MAX] = {0};
    int i, j, found, count = 0;
    char result[8192] = "";

    if (!read_matrices()) {
        write_output("Invalid input. Enter non-negative integers in every cell.");
        return;
    }

    for (j = 0; j < m; j++)
        work[j] = available[j];

    for (i = 0; i < n; i++) {
        int holds_resources = 0;

        for (j = 0; j < m; j++) {
            if (allocation[i][j] != 0)
                holds_resources = 1;
        }

        if (!holds_resources)
            finish[i] = 1;
    }

    do {
        found = 0;

        for (i = 0; i < n; i++) {
            if (finish[i])
                continue;

            int possible = 1;

            for (j = 0; j < m; j++) {
                if (request[i][j] > work[j]) {
                    possible = 0;
                    break;
                }
            }

            if (possible) {
                for (j = 0; j < m; j++)
                    work[j] += allocation[i][j];

                finish[i] = 1;
                found = 1;
            }
        }
    } while (found);

    for (i = 0; i < n; i++) {
        deadlocked[i] = !finish[i];

        if (deadlocked[i]) {
            char line[100];

            snprintf(line, sizeof(line),
                     "P%d is deadlocked.\n", i + 1);
            strncat(result, line,
                    sizeof(result) - strlen(result) - 1);
            count++;
        }
    }

    if (count == 0)
        strncat(result, "No deadlock detected.\n",
                sizeof(result) - strlen(result) - 1);
    else {
        char line[100];

        snprintf(line, sizeof(line),
                 "Total deadlocked processes: %d\n", count);
        strncat(result, line,
                sizeof(result) - strlen(result) - 1);
    }

    write_output(result);
}

void generate_rag(void)
{
    char result[8192] = "";
    char line[150];
    int i, j;

    if (!read_matrices()) {
        write_output("Invalid input. Enter non-negative integers in every cell.");
        return;
    }

    strncat(result, "RESOURCE ALLOCATION GRAPH (RAG)\n\n",
            sizeof(result) - strlen(result) - 1);

    strncat(result, "Allocation edges (Resource -> Process):\n",
            sizeof(result) - strlen(result) - 1);

    for (i = 0; i < n; i++) {
        for (j = 0; j < m; j++) {
            if (allocation[i][j] > 0) {
                snprintf(line, sizeof(line),
                         "R%d -> P%d (units: %d)\n",
                         j + 1, i + 1, allocation[i][j]);

                strncat(result, line,
                        sizeof(result) - strlen(result) - 1);
            }
        }
    }

    strncat(result, "\nRequest edges (Process -> Resource):\n",
            sizeof(result) - strlen(result) - 1);

    for (i = 0; i < n; i++) {
        for (j = 0; j < m; j++) {
            if (request[i][j] > 0) {
                snprintf(line, sizeof(line),
                         "P%d -> R%d (units: %d)\n",
                         i + 1, j + 1, request[i][j]);

                strncat(result, line,
                        sizeof(result) - strlen(result) - 1);
            }
        }
    }

    write_output(result);
}

void recover_deadlock(void)
{
    int p, j;
    char line[200];

    if (!read_matrices()) {
        write_output("Invalid input. Enter non-negative integers in every cell.");
        return;
    }

    p = gtk_spin_button_get_value_as_int(
        GTK_SPIN_BUTTON(selected_spin)) - 1;

    detect_deadlock();

    if (!deadlocked[p]) {
        append_output("Recovery cancelled: the selected process is not deadlocked.");
        return;
    }

    snprintf(line, sizeof(line),
             "\nTerminating P%d and releasing its resources...\n", p + 1);
    append_output(line);

    for (j = 0; j < m; j++) {
        available[j] += allocation[p][j];
        allocation[p][j] = 0;
        request[p][j] = 0;
    }

    for (j = 0; j < m; j++) {
        char value[20];

        snprintf(value, sizeof(value), "%d", available[j]);
        gtk_entry_set_text(GTK_ENTRY(available_entries[j]), value);

        snprintf(value, sizeof(value), "%d", allocation[p][j]);
        gtk_entry_set_text(GTK_ENTRY(allocation_entries[p][j]), value);

        snprintf(value, sizeof(value), "%d", request[p][j]);
        gtk_entry_set_text(GTK_ENTRY(request_entries[p][j]), value);
    }

    append_output("Resources released. Checking deadlock status again...");
    detect_deadlock();
    append_output("Recovery operation completed.");
}

void load_demo(void)
{
    int i, j;

    gtk_spin_button_set_value(GTK_SPIN_BUTTON(process_spin), 2);
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(resource_spin), 2);
    create_matrices();

    for (j = 0; j < m; j++) {
        gtk_entry_set_text(GTK_ENTRY(available_entries[j]), "0");
    }

    int alloc[2][2] = {{1, 0}, {0, 1}};
    int req[2][2] = {{0, 1}, {1, 0}};

    for (i = 0; i < 2; i++) {
        for (j = 0; j < 2; j++) {
            char a[10], r[10];

            snprintf(a, sizeof(a), "%d", alloc[i][j]);
            snprintf(r, sizeof(r), "%d", req[i][j]);

            gtk_entry_set_text(
                GTK_ENTRY(allocation_entries[i][j]), a);
            gtk_entry_set_text(
                GTK_ENTRY(request_entries[i][j]), r);
        }
    }

    gtk_spin_button_set_value(GTK_SPIN_BUTTON(selected_spin), 2);

    write_output(
        "Demo loaded.\n"
        "P1 holds R1 and requests R2.\n"
        "P2 holds R2 and requests R1.\n"
        "Click Detect deadlock to test the example.");
}

void clear_output(void)
{
    write_output("");
}

GtkWidget *make_button(const char *label,
                       GCallback callback,
                       gpointer data)
{
    GtkWidget *button = gtk_button_new_with_label(label);

    g_signal_connect(button, "clicked", callback, data);
    return button;
}

void on_create_clicked(GtkButton *button, gpointer data)
{
    create_matrices();
}

void on_detect_clicked(GtkButton *button, gpointer data)
{
    detect_deadlock();
}

void on_graph_clicked(GtkButton *button, gpointer data)
{
    generate_rag();
}

void on_recover_clicked(GtkButton *button, gpointer data)
{
    recover_deadlock();
}

void on_demo_clicked(GtkButton *button, gpointer data)
{
    load_demo();
}

void on_clear_clicked(GtkButton *button, gpointer data)
{
    clear_output();
}

int main(int argc, char *argv[])
{
    GtkWidget *root_box, *setup_frame, *setup_grid;
    GtkWidget *matrix_box, *available_frame;
    GtkWidget *allocation_frame, *request_frame;
    GtkWidget *actions_frame, *actions_box;
    GtkWidget *output_frame, *scroll;
    GtkWidget *title, *subtitle, *top_box;

    gtk_init(&argc, &argv);

    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window),
                         "Deadlock Detection and Recovery - Student 3");
    gtk_window_set_default_size(GTK_WINDOW(window), 980, 760);
    gtk_container_set_border_width(GTK_CONTAINER(window), 12);

    g_signal_connect(window, "destroy",
                     G_CALLBACK(gtk_main_quit), NULL);

    root_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 8);
    gtk_container_add(GTK_CONTAINER(window), root_box);

    top_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 3);
    title = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(title),
        "<span size='x-large' weight='bold'>Deadlock Detection and Recovery</span>");
    gtk_widget_set_halign(title, GTK_ALIGN_START);

    subtitle = make_label("Student 3 module | C algorithm + GTK GUI");
    gtk_widget_set_halign(subtitle, GTK_ALIGN_START);

    gtk_box_pack_start(GTK_BOX(top_box), title, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(top_box), subtitle, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(root_box), top_box, FALSE, FALSE, 5);

    setup_frame = gtk_frame_new("1. System setup");
    setup_grid = gtk_grid_new();
    gtk_grid_set_row_spacing(GTK_GRID(setup_grid), 8);
    gtk_grid_set_column_spacing(GTK_GRID(setup_grid), 8);
    gtk_container_set_border_width(GTK_CONTAINER(setup_grid), 8);
    gtk_container_add(GTK_CONTAINER(setup_frame), setup_grid);

    process_spin = gtk_spin_button_new_with_range(1, MAX, 2);
    resource_spin = gtk_spin_button_new_with_range(1, MAX, 2);
    selected_spin = gtk_spin_button_new_with_range(1, 2, 2);

    gtk_grid_attach(GTK_GRID(setup_grid),
                    make_label("Processes (1-20):"), 0, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(setup_grid), process_spin, 1, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(setup_grid),
                    make_label("Resource types (1-20):"), 2, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(setup_grid), resource_spin, 3, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(setup_grid),
                    make_button("Create / Reset matrices",
                                G_CALLBACK(on_create_clicked), NULL),
                    4, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(setup_grid),
                    make_button("Load deadlock demo",
                                G_CALLBACK(on_demo_clicked), NULL),
                    5, 0, 1, 1);

    gtk_box_pack_start(GTK_BOX(root_box), setup_frame, FALSE, FALSE, 0);

    matrix_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 6);
    available_frame = gtk_frame_new("Available vector");
    allocation_frame = gtk_frame_new("Allocation matrix");
    request_frame = gtk_frame_new("Request matrix");

    available_grid = gtk_grid_new();
    allocation_grid = gtk_grid_new();
    request_grid = gtk_grid_new();

    gtk_grid_set_row_spacing(GTK_GRID(available_grid), 4);
    gtk_grid_set_column_spacing(GTK_GRID(available_grid), 6);
    gtk_grid_set_row_spacing(GTK_GRID(allocation_grid), 4);
    gtk_grid_set_column_spacing(GTK_GRID(allocation_grid), 6);
    gtk_grid_set_row_spacing(GTK_GRID(request_grid), 4);
    gtk_grid_set_column_spacing(GTK_GRID(request_grid), 6);

    gtk_container_set_border_width(GTK_CONTAINER(available_grid), 8);
    gtk_container_set_border_width(GTK_CONTAINER(allocation_grid), 8);
    gtk_container_set_border_width(GTK_CONTAINER(request_grid), 8);

    gtk_container_add(GTK_CONTAINER(available_frame), available_grid);
    gtk_container_add(GTK_CONTAINER(allocation_frame), allocation_grid);
    gtk_container_add(GTK_CONTAINER(request_frame), request_grid);

    gtk_box_pack_start(GTK_BOX(matrix_box), available_frame,
                       FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(matrix_box), allocation_frame,
                       FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(matrix_box), request_frame,
                       FALSE, FALSE, 0);

    scroll = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll),
                                   GTK_POLICY_AUTOMATIC,
                                   GTK_POLICY_AUTOMATIC);
    gtk_container_add(GTK_CONTAINER(scroll), matrix_box);
    gtk_widget_set_size_request(scroll, -1, 280);
    gtk_box_pack_start(GTK_BOX(root_box), scroll, TRUE, TRUE, 0);

    actions_frame = gtk_frame_new("2. Run operation");
    actions_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 6);
    gtk_container_set_border_width(GTK_CONTAINER(actions_box), 8);
    gtk_container_add(GTK_CONTAINER(actions_frame), actions_box);

    gtk_box_pack_start(GTK_BOX(actions_box),
        make_button("Detect deadlock", G_CALLBACK(on_detect_clicked), NULL),
        FALSE, FALSE, 0);

    gtk_box_pack_start(GTK_BOX(actions_box),
        make_button("Generate RAG data", G_CALLBACK(on_graph_clicked), NULL),
        FALSE, FALSE, 0);

    gtk_box_pack_start(GTK_BOX(actions_box),
        make_label("Terminate process:"), FALSE, FALSE, 0);

    gtk_box_pack_start(GTK_BOX(actions_box), selected_spin,
                       FALSE, FALSE, 0);

    gtk_box_pack_start(GTK_BOX(actions_box),
        make_button("Recover", G_CALLBACK(on_recover_clicked), NULL),
        FALSE, FALSE, 0);

    gtk_box_pack_end(GTK_BOX(actions_box),
        make_button("Clear output", G_CALLBACK(on_clear_clicked), NULL),
        FALSE, FALSE, 0);

    gtk_box_pack_start(GTK_BOX(root_box), actions_frame,
                       FALSE, FALSE, 0);

    output_frame = gtk_frame_new("3. Results");
    output_view = gtk_text_view_new();
    gtk_text_view_set_editable(GTK_TEXT_VIEW(output_view), FALSE);
    gtk_text_view_set_cursor_visible(GTK_TEXT_VIEW(output_view), FALSE);
    gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(output_view), GTK_WRAP_WORD);
    gtk_text_view_set_monospace(GTK_TEXT_VIEW(output_view), TRUE);

    scroll = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll),
                                   GTK_POLICY_AUTOMATIC,
                                   GTK_POLICY_AUTOMATIC);
    gtk_container_add(GTK_CONTAINER(scroll), output_view);
    gtk_container_add(GTK_CONTAINER(output_frame), scroll);

    gtk_box_pack_start(GTK_BOX(root_box), output_frame,
                       TRUE, TRUE, 0);

    create_matrices();
    write_output("Enter resource values, or click Load deadlock demo.");

    gtk_widget_show_all(window);
    gtk_main();

    return 0;
}
