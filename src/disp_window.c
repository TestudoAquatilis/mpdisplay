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
    w->win_main    = NULL;
    w->img_play    = NULL;
    w->pbar_time   = NULL;
    w->lbar_volume = NULL;
    w->tb_shuffle  = NULL;
    w->tb_repeat   = NULL;
    w->tb_consec   = NULL;

    /* temporaryly stored widgets */
    GtkWidget *vbox0       = NULL;
    GtkWidget *vbox0_s0    = NULL;
    GtkWidget *vbox0_s1    = NULL;
    GtkWidget *hbox1p      = NULL;
    GtkWidget *hbox1s      = NULL;
    GtkWidget *img_consec  = NULL;
    GtkWidget *img_shuffle = NULL;
    GtkWidget *img_repeat  = NULL;
    GtkWidget *img_volume  = NULL;

    /***************/
    /* gtk widgets */
    w->win_main    = gtk_window_new (GTK_WINDOW_TOPLEVEL);
    vbox0          = gtk_box_new (GTK_ORIENTATION_VERTICAL, 1);
    vbox0_s0       = gtk_separator_new (GTK_ORIENTATION_HORIZONTAL);
    vbox0_s1       = gtk_separator_new (GTK_ORIENTATION_HORIZONTAL);
    hbox1p         = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 1);
    w->img_play    = gtk_image_new_from_icon_name ("media-playback-stop-symbolic", GTK_ICON_SIZE_DIALOG);
    w->pbar_time   = gtk_progress_bar_new ();
    w->lbar_volume = gtk_level_bar_new_for_interval (0, 100);
    hbox1s         = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 1);
    w->tb_consec   = gtk_toggle_button_new ();
    w->tb_shuffle  = gtk_toggle_button_new ();
    w->tb_repeat   = gtk_toggle_button_new ();
    img_consec     = gtk_image_new_from_icon_name ("media-playlist-consecutive-symbolic", GTK_ICON_SIZE_SMALL_TOOLBAR);
    img_shuffle    = gtk_image_new_from_icon_name ("media-playlist-shuffle-symbolic", GTK_ICON_SIZE_SMALL_TOOLBAR);
    img_repeat     = gtk_image_new_from_icon_name ("media-playlist-repeat-symbolic", GTK_ICON_SIZE_SMALL_TOOLBAR);
    img_volume     = gtk_image_new_from_icon_name ("multimedia-volume-control-symbolic", GTK_ICON_SIZE_SMALL_TOOLBAR);

    /************/
    /* settings */
    gtk_window_set_title (GTK_WINDOW (w->win_main), "MPDisplay");

    gtk_progress_bar_set_fraction  (GTK_PROGRESS_BAR (w->pbar_time), 0.0);
    gtk_progress_bar_set_text      (GTK_PROGRESS_BAR (w->pbar_time), "");
    gtk_progress_bar_set_show_text (GTK_PROGRESS_BAR (w->pbar_time), true);

    gtk_orientable_set_orientation (GTK_ORIENTABLE (w->lbar_volume), GTK_ORIENTATION_HORIZONTAL);

    /***************/
    /* arrangement */
    gtk_box_pack_start (GTK_BOX (hbox1p), w->img_play,  false, true, 0);
    gtk_box_pack_start (GTK_BOX (hbox1p), w->pbar_time, true,  true, 0);

    gtk_container_add (GTK_CONTAINER (w->tb_consec),  img_consec);
    gtk_container_add (GTK_CONTAINER (w->tb_shuffle), img_shuffle);
    gtk_container_add (GTK_CONTAINER (w->tb_repeat),  img_repeat);

    gtk_box_pack_start (GTK_BOX (hbox1s), w->tb_consec,   false, true, 0);
    gtk_box_pack_start (GTK_BOX (hbox1s), w->tb_shuffle,  false, true, 0);
    gtk_box_pack_start (GTK_BOX (hbox1s), w->tb_repeat,   false, true, 0);
    gtk_box_pack_end   (GTK_BOX (hbox1s), w->lbar_volume, false, true, 0);
    gtk_box_pack_end   (GTK_BOX (hbox1s), img_volume,     false, true, 0);

    gtk_box_pack_start (GTK_BOX (vbox0), hbox1p,   false, true, 0);
    gtk_box_pack_start (GTK_BOX (vbox0), vbox0_s0, false, true, 0);
    gtk_box_pack_start (GTK_BOX (vbox0), vbox0_s1, false, true, 0);
    gtk_box_pack_start (GTK_BOX (vbox0), hbox1s,   false, true, 0);

    gtk_container_add (GTK_CONTAINER (w->win_main), vbox0);

    /**********/
    /* events */
    g_signal_connect (G_OBJECT (w->win_main), "destroy", G_CALLBACK (sh_window_close), (gpointer) w);

    return w;
}

void disp_window_free (struct disp_window **w_p)
{
    if (w_p == NULL) return;

    struct disp_window *w = *w_p;

    g_slice_free (struct disp_window, w);
    *w_p = NULL;
}

void disp_window_show (struct disp_window *w)
{
    if (w == NULL) return;

    gtk_widget_show_all (GTK_WIDGET (w->win_main));
}

void disp_window_update (struct disp_window *w, struct mpd_status *s)
{
    if ((s == NULL) || (w == NULL)) return;

    /* play/pause/stop icon */
    if (s->play) {
        gtk_image_set_from_icon_name (GTK_IMAGE (w->img_play), "media-playback-play-symbolic", GTK_ICON_SIZE_DIALOG);
    } else if (s->pause) {
        gtk_image_set_from_icon_name (GTK_IMAGE (w->img_play), "media-playback-pause-symbolic", GTK_ICON_SIZE_DIALOG);
    } else {
        gtk_image_set_from_icon_name (GTK_IMAGE (w->img_play), "media-playback-stop-symbolic", GTK_ICON_SIZE_DIALOG);
    }

    /* playback time */
    gdouble  pb_progress = 0;
    bool     pb_pulse    = false;
    GString *st_progress = g_string_new (NULL);

    if (s->play || s->pause) {
        if ((s->seconds_total > 0) && (s->seconds_elapsed >= 0) && (s->seconds_total >= s->seconds_elapsed)) {
            pb_progress = (gdouble) s->seconds_elapsed / (gdouble) s->seconds_total;
        } else {
            pb_progress = 1;
            if (s->play) pb_pulse = true;
        }
        if (s->seconds_elapsed >= 0) {
            g_string_printf (st_progress, "%d:%02d", s->seconds_elapsed/60, s->seconds_elapsed%60);
        } else {
            st_progress = g_string_assign (st_progress, "?");
        }
        if (s->seconds_total > 0) {
            g_string_append_printf (st_progress, " / %d:%02d", s->seconds_total/60, s->seconds_total%60);
        }
    } else {
        if (s->seconds_total >= 0) {
            g_string_printf (st_progress, "%d:%02d", s->seconds_total/60, s->seconds_total%60);
        } else {
            st_progress = g_string_assign (st_progress, "");
        }
    }

    gtk_progress_bar_set_fraction  (GTK_PROGRESS_BAR (w->pbar_time), pb_progress);
    gtk_progress_bar_set_text      (GTK_PROGRESS_BAR (w->pbar_time), st_progress->str);

    if (pb_pulse) gtk_progress_bar_pulse (GTK_PROGRESS_BAR (w->pbar_time));

    g_string_free (st_progress, true);

    /* playlist status */
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (w->tb_consec),  !s->single);
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (w->tb_shuffle), s->shuffle);
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (w->tb_repeat),  s->repeat);

    /* volume */
    int volume = 0;
    if (s->volume >= 0) volume = s->volume;
    if (volume > 100)   volume = 100;
    gtk_level_bar_set_value (GTK_LEVEL_BAR (w->lbar_volume), volume);
}

/* signal handlers */
static void sh_window_close (GtkWidget *widget, gpointer data)
{
    gtk_main_quit ();

    struct disp_window* w = (struct disp_window*) data;
    w->win_main = NULL;
}
