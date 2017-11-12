#include <glib.h>
#include <gtk/gtk.h>

#include "disp_window.h"

/* signal handler declaration */
static void sh_window_close (GtkWidget *widget, gpointer data);

/* initialization/finalization */
struct disp_window *disp_window_new ()
{
    struct disp_window *w = g_slice_new (struct disp_window);
    if (w == NULL) return NULL;

    /* gtk widgets */
    w->window = gtk_window_new (GTK_WINDOW_TOPLEVEL);

    /* settings */
    gtk_window_set_title (GTK_WINDOW (w->window), "MPDisplay");

    /* events */
	g_signal_connect (G_OBJECT (w->window), "destroy", G_CALLBACK (sh_window_close), (gpointer) w);

    return w;
}

void disp_window_free (struct disp_window **win)
{
    if (win == NULL) return;

    struct disp_window *w = *win;

    g_slice_free (struct disp_window, w);
    *win = NULL;
}

void disp_window_show (struct disp_window *w)
{
    gtk_widget_show_all (GTK_WIDGET (w->window));
}

/* signal handlers */
static void sh_window_close (GtkWidget *widget, gpointer data)
{
	gtk_main_quit ();

    struct disp_window* w = (struct disp_window*) data;
    w->window = NULL;
}
