
#include <gtk/gtk.h>
#include "gtk_module.h"

static Student2GtkState state;

static void activate(GtkApplication *app, gpointer user_data)
{
    (void)user_data;

    GtkWidget *window = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(window),
                         "Resource Management System");
    gtk_window_set_default_size(GTK_WINDOW(window), 900, 650);

    GtkWidget *main_box =
        gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_container_set_border_width(GTK_CONTAINER(main_box), 15);
    gtk_container_add(GTK_CONTAINER(window), main_box);

    GtkWidget *heading =
        gtk_label_new("Resource Management & Banker's Algorithm");
    gtk_box_pack_start(GTK_BOX(main_box), heading,
                       FALSE, FALSE, 5);

    GtkWidget *button_grid = gtk_grid_new();
    gtk_grid_set_row_spacing(GTK_GRID(button_grid), 8);
    gtk_grid_set_column_spacing(GTK_GRID(button_grid), 8);
    gtk_box_pack_start(GTK_BOX(main_box), button_grid,
                       FALSE, FALSE, 5);

    const char *button_names[] = {
        "Create Process",
        "Create Resource",
        "Set Allocation",
        "Set Maximum",
        "Set Request",
        "Submit Resource Request",
        "Run Banker's Algorithm",
        "System Status"
    };

    void (*callbacks[])(GtkWidget *, gpointer) = {
        student2_gtk_create_process,
        student2_gtk_create_resource,
        student2_gtk_set_allocation,
        student2_gtk_set_maximum,
        student2_gtk_set_request,
        student2_gtk_submit_request,
        student2_gtk_run_bankers,
        student2_gtk_system_status
    };

    for (int i = 0; i < 8; i++) {
        GtkWidget *button = gtk_button_new_with_label(
            button_names[i]);

        g_signal_connect(button, "clicked",
                         G_CALLBACK(callbacks[i]), &state);

        gtk_grid_attach(GTK_GRID(button_grid), button,
                        i % 2, i / 2, 1, 1);
    }

    GtkWidget *matrix_heading =
        gtk_label_new("System Matrices");
    gtk_box_pack_start(GTK_BOX(main_box), matrix_heading,
                       FALSE, FALSE, 3);

    GtkWidget *scroll =
        gtk_scrolled_window_new(NULL, NULL);
    gtk_widget_set_vexpand(scroll, TRUE);
    gtk_box_pack_start(GTK_BOX(main_box), scroll,
                       TRUE, TRUE, 5);

    GtkWidget *matrix_view = gtk_text_view_new();
    gtk_text_view_set_editable(GTK_TEXT_VIEW(matrix_view), FALSE);
    gtk_text_view_set_cursor_visible(GTK_TEXT_VIEW(matrix_view), FALSE);
    gtk_text_view_set_monospace(GTK_TEXT_VIEW(matrix_view), TRUE);
    gtk_container_add(GTK_CONTAINER(scroll), matrix_view);

    GtkWidget *status_label =
        gtk_label_new("Add processes and resources to begin.");
    gtk_box_pack_start(GTK_BOX(main_box), status_label,
                       FALSE, FALSE, 5);

    student2_gtk_init(
        &state,
        window,
        gtk_text_view_get_buffer(GTK_TEXT_VIEW(matrix_view)),
        GTK_LABEL(status_label)
    );

    gtk_widget_show_all(window);
}

int main(int argc, char **argv)
{
    GtkApplication *app = gtk_application_new(
        "edu.resource.management",
        G_APPLICATION_DEFAULT_FLAGS);

    g_signal_connect(app, "activate",
                     G_CALLBACK(activate), NULL);

    int status = g_application_run(
        G_APPLICATION(app), argc, argv);

    g_object_unref(app);
    return status;
}
