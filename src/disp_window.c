#include <glib.h>
#include <gtk/gtk.h>
#include <stdbool.h>

#include "disp_window.h"

/* signal handler declaration */
static void sh_window_close (GtkWidget *widget, gpointer data);

/* initialization/finalization */
struct disp_window *disp_window_new ()
{
    struct disp_window *w = g_slice_new (struct disp_window);
    if (w == NULL) return NULL;

    /*****************/
    /* init pointers */
    w->window    = NULL;
    w->vbox0     = NULL;
    w->vbox0_s0  = NULL;
    w->vbox0_s1  = NULL;
    w->hbox1p    = NULL;
    w->img_play  = NULL;
    w->pbar_time = NULL;
    w->hbox1s    = NULL;

    /***************/
    /* gtk widgets */
    w->window    = gtk_window_new (GTK_WINDOW_TOPLEVEL);
    w->vbox0     = gtk_box_new (GTK_ORIENTATION_VERTICAL, 1);
    w->vbox0_s0  = gtk_separator_new (GTK_ORIENTATION_HORIZONTAL);
    w->vbox0_s1  = gtk_separator_new (GTK_ORIENTATION_HORIZONTAL);
    w->hbox1p    = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 1);
    w->img_play  = gtk_image_new_from_icon_name ("media-playback-stop", GTK_ICON_SIZE_DIALOG);
    w->pbar_time = gtk_progress_bar_new ();
    w->hbox1s    = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 1);

    /************/
    /* settings */
    gtk_window_set_title (GTK_WINDOW (w->window), "MPDisplay");

    gtk_progress_bar_set_fraction  (GTK_PROGRESS_BAR (w->pbar_time), 0.0);
    gtk_progress_bar_set_text      (GTK_PROGRESS_BAR (w->pbar_time), "");
    gtk_progress_bar_set_show_text (GTK_PROGRESS_BAR (w->pbar_time), true);

    /***************/
    /* arrangement */
    gtk_box_pack_start (GTK_BOX (w->hbox1p), w->img_play,  false, true, 0);
    gtk_box_pack_start (GTK_BOX (w->hbox1p), w->pbar_time, true,  true, 0);

    gtk_box_pack_start (GTK_BOX (w->vbox0), w->hbox1p,   false, true, 0);
    gtk_box_pack_start (GTK_BOX (w->vbox0), w->vbox0_s0, false, true, 0);
    gtk_box_pack_start (GTK_BOX (w->vbox0), w->vbox0_s1, false, true, 0);
    gtk_box_pack_start (GTK_BOX (w->vbox0), w->hbox1s,   false, true, 0);

    gtk_container_add (GTK_CONTAINER (w->window), w->vbox0);

    /**********/
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
