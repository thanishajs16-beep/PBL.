#ifndef STUDENT2_GTK_MODULE_H
#define STUDENT2_GTK_MODULE_H

#include <gtk/gtk.h>

#define S2_MAX_PROCESSES 20
#define S2_MAX_RESOURCES 10
#define S2_NAME_LEN 32

typedef struct {
    int process_count;
    int resource_count;
    char process_names[S2_MAX_PROCESSES][S2_NAME_LEN];
    char resource_names[S2_MAX_RESOURCES][S2_NAME_LEN];
    int total[S2_MAX_RESOURCES];
    int available[S2_MAX_RESOURCES];
    int allocation[S2_MAX_PROCESSES][S2_MAX_RESOURCES];
    int maximum[S2_MAX_PROCESSES][S2_MAX_RESOURCES];
    int need[S2_MAX_PROCESSES][S2_MAX_RESOURCES];
    int request[S2_MAX_PROCESSES][S2_MAX_RESOURCES];

    GtkWidget *parent_window;
    GtkTextBuffer *matrix_buffer;
    GtkLabel *status_label; /* May be NULL */
} Student2GtkState;

void student2_gtk_init(Student2GtkState *state,
                       GtkWidget *parent_window,
                       GtkTextBuffer *matrix_buffer,
                       GtkLabel *status_label);
void student2_gtk_refresh_matrices(Student2GtkState *state);

void student2_gtk_create_process(GtkWidget *button, gpointer user_data);
void student2_gtk_create_resource(GtkWidget *button, gpointer user_data);
void student2_gtk_set_allocation(GtkWidget *button, gpointer user_data);
void student2_gtk_set_maximum(GtkWidget *button, gpointer user_data);
void student2_gtk_set_request(GtkWidget *button, gpointer user_data);
void student2_gtk_submit_request(GtkWidget *button, gpointer user_data);
void student2_gtk_run_bankers(GtkWidget *button, gpointer user_data);
void student2_gtk_system_status(GtkWidget *button, gpointer user_data);

#endif
