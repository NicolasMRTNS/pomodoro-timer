#include <stdio.h>
#include <gtk/gtk.h>

#define DEFAULT_WORK_MINUTES 25
#define DEFAULT_BREAK_MINUTES 5

int seconds_left = DEFAULT_WORK_MINUTES * 60;
gboolean is_running = FALSE;
gboolean on_break = FALSE;
GtkWidget *label;

gboolean update_timer(gpointer user_data) {
    if (is_running && seconds_left > 0) {
        seconds_left--;
        const int minutes = seconds_left / 60;
        const int seconds =  seconds_left % 60;
        char time_str[10];
        snprintf(time_str, sizeof(time_str), "%02d:%02d", minutes, seconds);
        gtk_label_set_text(GTK_LABEL(label), time_str);
        return TRUE;
    }

    if (seconds_left == 0 && is_running) {
        is_running = FALSE;
        gtk_label_set_text(GTK_LABEL(label), on_break ? "Break Over!" : "Time's Up!");
        on_break = !on_break;
    }

    return TRUE;
}

void start_timer(GtkWidget *widget, gpointer data) {
    if (!is_running && seconds_left > 0) {
        is_running = TRUE;
    }
}

void pause_timer(GtkWidget *widget, gpointer data) {
    is_running = FALSE;
}

void reset_timer(GtkWidget *widget, gpointer data) {
    is_running = FALSE;
    on_break = FALSE;
    seconds_left = DEFAULT_WORK_MINUTES * 60;
    gtk_label_set_text(GTK_LABEL(label), "25:00");
}

int main(int argc, char *argv[]) {
    gtk_init(&argc, &argv);

    GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Pomodoro Timer");
    gtk_window_set_default_size(GTK_WINDOW(window), 200, 100);
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    GtkWidget *grid = gtk_grid_new();
    gtk_container_add(GTK_CONTAINER(window), grid);

    label = gtk_label_new("25:00");
    gtk_widget_set_halign(label, GTK_ALIGN_CENTER);
    gtk_widget_set_valign(label, GTK_ALIGN_CENTER);
    gtk_grid_attach(GTK_GRID(grid), label, 0, 0, 3, 1);

    GtkWidget *start_button = gtk_button_new_with_label("Start");
    g_signal_connect(start_button, "clicked", G_CALLBACK(start_timer), NULL);
    gtk_grid_attach(GTK_GRID(grid), start_button, 0, 1, 1, 1);

    GtkWidget *pause_button = gtk_button_new_with_label("Pause");
    g_signal_connect(pause_button, "clicked", G_CALLBACK(pause_timer), NULL);
    gtk_grid_attach(GTK_GRID(grid), pause_button, 1, 1, 1, 1);

    GtkWidget *reset_button = gtk_button_new_with_label("Reset");
    g_signal_connect(reset_button, "clicked", G_CALLBACK(reset_timer), NULL);
    gtk_grid_attach(GTK_GRID(grid), reset_button, 2, 1, 1, 1);

    g_timeout_add_seconds(1, update_timer, NULL);

    gtk_widget_show_all(window);
    gtk_main();

    return 0;
}