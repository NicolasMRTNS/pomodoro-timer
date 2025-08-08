#include <stdio.h>
#include <gtk/gtk.h>

#define DEFAULT_WORK_MINUTES 25
#define DEFAULT_BREAK_MINUTES 5

typedef struct {
    char ticket[100];
    char description[256];
    int seconds_spent;
    GtkWidget *row_widget; // store pointer to its label for easy update
} Task;

static GList *tasks = NULL;
static Task *current_task = NULL;

static int seconds_left = DEFAULT_WORK_MINUTES * 60;
static gboolean is_running = FALSE;
static gboolean on_break = FALSE;

static GtkWidget *label_timer;
static GtkWidget *entry_ticket;
static GtkWidget *entry_desc;
static GtkWidget *list_box;

// Update all task labels, bold the active one
void update_task_labels() {
    for (const GList *l = tasks; l != NULL; l = l->next) {
        Task *task = l->data;
        char row_text[512];
        snprintf(row_text, sizeof(row_text), "%s — %d min — %s",
                 task->ticket, task->seconds_spent / 60, task->description);

        if (task == current_task) {
            char bold_text[600];
            snprintf(bold_text, sizeof(bold_text), "<b>%s</b>", row_text);
            gtk_label_set_markup(GTK_LABEL(task->row_widget), bold_text);
        } else {
            gtk_label_set_text(GTK_LABEL(task->row_widget), row_text);
        }
    }
}

// Update the timer every second
gboolean update_timer(gpointer user_data) {
    if (is_running && seconds_left > 0) {
        seconds_left--;

        // Update active task time
        if (current_task) {
            current_task->seconds_spent++;
        }

        update_task_labels();

        const int minutes = seconds_left / 60;
        const int seconds =  seconds_left % 60;
        char time_str[10];
        snprintf(time_str, sizeof(time_str), "%02d:%02d", minutes, seconds);
        gtk_label_set_text(GTK_LABEL(label_timer), time_str);
        return TRUE;
    }

    if (seconds_left == 0 && is_running) {
        is_running = FALSE;
        gtk_label_set_text(GTK_LABEL(label_timer), on_break ? "Break Over!" : "Time's Up!");
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
    gtk_label_set_text(GTK_LABEL(label_timer), "25:00");
}

// Task selection callback
void on_task_selected(GtkListBox *box, GtkListBoxRow *row, gpointer user_data) {
    if (!row) return;

    // Find the Task for this row
    for (const GList *l = tasks; l != NULL; l = l->next) {
        Task *task = l->data;
        if (gtk_widget_is_ancestor(GTK_WIDGET(row), task->row_widget)) {
            current_task = task;
            break;
        }
    }
    update_task_labels();
}

// Add a task
void add_task(GtkWidget *widget, gpointer data) {
    const char *ticket_text = gtk_entry_get_text(GTK_ENTRY(entry_ticket));
    const char *desc_text = gtk_entry_get_text(GTK_ENTRY(entry_desc));

    if (strlen(ticket_text) == 0) return;

    Task *new_task = g_malloc(sizeof(Task));
    strncpy(new_task->ticket, ticket_text, sizeof(new_task->ticket) - 1);
    strncpy(new_task->description, desc_text, sizeof(new_task->description) - 1);
    new_task->seconds_spent = 0;

    // Create row widget
    char row_text[512];
    snprintf(row_text, sizeof(row_text), "%s - 0 min - %s", new_task->ticket, new_task->description);
    GtkWidget *row_label = gtk_label_new(row_text);
    gtk_widget_set_halign(row_label, GTK_ALIGN_START);

    GtkWidget *row = gtk_list_box_row_new();
    gtk_container_add(GTK_CONTAINER(row), row_label);
    gtk_list_box_insert(GTK_LIST_BOX(list_box), row, -1);

    gtk_widget_show_all(list_box);

    new_task->row_widget = row_label;
    tasks = g_list_append(tasks, new_task);

    // Make the newly added task active
    current_task = new_task;
    gtk_list_box_select_row(GTK_LIST_BOX(list_box), GTK_LIST_BOX_ROW(row));
    update_task_labels();

    // Clear inputs
    gtk_entry_set_text(GTK_ENTRY(entry_ticket), "");
    gtk_entry_set_text(GTK_ENTRY(entry_desc), "");
}

int main(int argc, char *argv[]) {
    gtk_init(&argc, &argv);

    GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Pomodoro Task Tracker");
    gtk_window_set_default_size(GTK_WINDOW(window), 200, 100);
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    GtkWidget *grid = gtk_grid_new();
    gtk_grid_set_row_spacing(GTK_GRID(grid), 5);
    gtk_grid_set_column_spacing(GTK_GRID(grid), 5);
    gtk_container_add(GTK_CONTAINER(window), grid);

    label_timer = gtk_label_new("25:00");
    gtk_widget_set_halign(label_timer, GTK_ALIGN_CENTER);
    gtk_widget_set_valign(label_timer, GTK_ALIGN_CENTER);
    gtk_grid_attach(GTK_GRID(grid), label_timer, 0, 0, 3, 1);

    GtkWidget *start_button = gtk_button_new_with_label("Start");
    g_signal_connect(start_button, "clicked", G_CALLBACK(start_timer), NULL);
    gtk_grid_attach(GTK_GRID(grid), start_button, 0, 1, 1, 1);

    GtkWidget *pause_button = gtk_button_new_with_label("Pause");
    g_signal_connect(pause_button, "clicked", G_CALLBACK(pause_timer), NULL);
    gtk_grid_attach(GTK_GRID(grid), pause_button, 1, 1, 1, 1);

    GtkWidget *reset_button = gtk_button_new_with_label("Reset");
    g_signal_connect(reset_button, "clicked", G_CALLBACK(reset_timer), NULL);
    gtk_grid_attach(GTK_GRID(grid), reset_button, 2, 1, 1, 1);

    // Task inputs
    GtkWidget *task_label = gtk_label_new("Ticket:");
    gtk_grid_attach(GTK_GRID(grid), task_label, 0, 2, 1, 1);

    entry_ticket = gtk_entry_new();
    gtk_grid_attach(GTK_GRID(grid), entry_ticket, 1,  2, 2, 1);

    GtkWidget *desc_label = gtk_label_new("Description:");
    gtk_grid_attach(GTK_GRID(grid), desc_label, 0, 3, 1, 1);

    entry_desc = gtk_entry_new();
    gtk_grid_attach(GTK_GRID(grid), entry_desc, 1, 3, 2, 1);

    GtkWidget *add_button = gtk_button_new_with_label("Add Task");
    g_signal_connect(add_button, "clicked", G_CALLBACK(add_task), NULL);
    gtk_grid_attach(GTK_GRID(grid), add_button, 0, 4, 3, 1);

    // Task list
    list_box = gtk_list_box_new();
    gtk_list_box_set_selection_mode(GTK_LIST_BOX(list_box), GTK_SELECTION_SINGLE);
    g_signal_connect(list_box, "row-selected", G_CALLBACK(on_task_selected), NULL);
    gtk_grid_attach(GTK_GRID(grid), list_box, 0, 5, 3, 1);

    // Timer update every second
    g_timeout_add_seconds(1, update_timer, NULL);

    gtk_widget_show_all(window);
    gtk_main();

    return 0;
}