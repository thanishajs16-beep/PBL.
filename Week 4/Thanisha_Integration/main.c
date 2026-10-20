#include <gtk/gtk.h>
#include "deadlock.h"

int run_deadlock_application(void)
{
    DeadlockSystem system;
    deadlock_system_init(&system);
    gtk_init(NULL, NULL);
    gui_run(&system);
    return 0;
}

int main(void)
{
    return run_deadlock_application();
}
