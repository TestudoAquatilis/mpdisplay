#include <glib.h>
#include <gtk/gtk.h>
#include <stdbool.h>

#include "gwin_disp.h"
#include "options.h"

/* signal handler declaration */
static void sh_window_close (GtkWidget *widget, gpointer data);
static void song_data_update (GtkWidget *widget, struct mpdisplay_mpd_status *s, struct mpdisplay_mpd_status *cs);

/* initialization/finalization */
struct win_disp *win_disp_new ()
{
    struct win_disp *w = g_slice_new (struct win_disp);
    if (w == NULL) return NULL;

    w->done = true;

    /*****************/
    /* init pointers */
    w->mpd_st_current = NULL;

    w->win_main   = NULL;
    w->im_state   = NULL;
    w->pb_time    = NULL;
    w->pb_volume  = NULL;
    w->tb_shuffle = NULL;
    w->tb_repeat  = NULL;
    w->tb_single  = NULL;
    w->fr_center  = NULL;

    /* temporaryly stored widgets */
    GtkWidget *vbox0       = NULL;
    GtkWidget *vbox0_s0    = NULL;
    GtkWidget *vbox0_s1    = NULL;
    GtkWidget *hbox1p      = NULL;
    GtkWidget *hbox1s      = NULL;
    GtkWidget *lbl_single  = NULL;
    GtkWidget *img_shuffle = NULL;
    GtkWidget *img_repeat  = NULL;
    GtkWidget *img_volume  = NULL;

    /***************/
    /* gtk widgets */
    w->win_main   = gtk_window_new (GTK_WINDOW_TOPLEVEL);
    vbox0         = gtk_box_new (GTK_ORIENTATION_VERTICAL, 1);
    vbox0_s0      = gtk_separator_new (GTK_ORIENTATION_HORIZONTAL);
    vbox0_s1      = gtk_separator_new (GTK_ORIENTATION_HORIZONTAL);
    hbox1p        = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 1);
    w->im_state   = gtk_image_new_from_icon_name ("media-playback-stop-symbolic", GTK_ICON_SIZE_DIALOG);
    w->pb_time    = gtk_progress_bar_new ();
    w->pb_volume  = gtk_progress_bar_new ();
    hbox1s        = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 1);
    w->tb_single  = gtk_toggle_button_new ();
    w->tb_shuffle = gtk_toggle_button_new ();
    w->tb_repeat  = gtk_toggle_button_new ();
    lbl_single    = gtk_label_new ("1");
    img_shuffle   = gtk_image_new_from_icon_name ("media-playlist-shuffle-symbolic", GTK_ICON_SIZE_SMALL_TOOLBAR);
    img_repeat    = gtk_image_new_from_icon_name ("media-playlist-repeat-symbolic", GTK_ICON_SIZE_SMALL_TOOLBAR);
    img_volume    = gtk_image_new_from_icon_name ("multimedia-volume-control-symbolic", GTK_ICON_SIZE_SMALL_TOOLBAR);
    w->fr_center  = gtk_frame_new (NULL);

    /************/
    /* settings */
    gtk_window_set_title (GTK_WINDOW (w->win_main), "MPDisplay");
    if ((mpdisplay_options.win_width > 0) | (mpdisplay_options.win_height > 0)) {
        int w_w = mpdisplay_options.win_width;
        int w_h = mpdisplay_options.win_height;
        gtk_window_set_default_size (GTK_WINDOW (w->win_main), w_w, w_h);
    }
    if (mpdisplay_options.win_fullscreen) {
        gtk_window_fullscreen (GTK_WINDOW (w->win_main));
    }

    gtk_progress_bar_set_fraction  (GTK_PROGRESS_BAR (w->pb_time), 0.0);
    gtk_progress_bar_set_text      (GTK_PROGRESS_BAR (w->pb_time), "");
    gtk_progress_bar_set_show_text (GTK_PROGRESS_BAR (w->pb_time), true);

    gtk_progress_bar_set_fraction  (GTK_PROGRESS_BAR (w->pb_volume), 0.0);

    gtk_frame_set_shadow_type (GTK_FRAME (w->fr_center), GTK_SHADOW_NONE);

    /***************/
    /* arrangement */
    gtk_box_pack_start (GTK_BOX (hbox1p), w->im_state,  false, true, 0);
    gtk_box_pack_start (GTK_BOX (hbox1p), w->pb_time, true,  true, 0);

    gtk_container_add (GTK_CONTAINER (w->tb_single),  lbl_single);
    gtk_container_add (GTK_CONTAINER (w->tb_shuffle), img_shuffle);
    gtk_container_add (GTK_CONTAINER (w->tb_repeat),  img_repeat);

    gtk_box_pack_start (GTK_BOX (hbox1s), w->tb_single,  false, true, 0);
    gtk_box_pack_start (GTK_BOX (hbox1s), w->tb_shuffle, false, true, 0);
    gtk_box_pack_start (GTK_BOX (hbox1s), w->tb_repeat,  false, true, 0);
    gtk_box_pack_end   (GTK_BOX (hbox1s), w->pb_volume,  false, true, 0);
    gtk_box_pack_end   (GTK_BOX (hbox1s), img_volume,    false, true, 0);

    gtk_box_pack_start (GTK_BOX (vbox0), hbox1p,       false, true, 0);
    gtk_box_pack_start (GTK_BOX (vbox0), vbox0_s0,     false, true, 0);
    gtk_box_pack_start (GTK_BOX (vbox0), w->fr_center, true,  true, 0);
    gtk_box_pack_start (GTK_BOX (vbox0), vbox0_s1,     false, true, 0);
    gtk_box_pack_start (GTK_BOX (vbox0), hbox1s,       false, true, 0);

    gtk_container_add (GTK_CONTAINER (w->win_main), vbox0);

    /**********/
    /* events */
    g_signal_connect (G_OBJECT (w->win_main), "destroy", G_CALLBACK (sh_window_close), (gpointer) w);
    w->done = false;

    return w;
}

void win_disp_free (struct win_disp **w_p)
{
    if (w_p == NULL) return;

    struct win_disp *w = *w_p;

    if (w == NULL) return;

    mpdisplay_mpd_status_free (&(w->mpd_st_current));

    g_slice_free (struct win_disp, w);
    *w_p = NULL;
}

void win_disp_show (struct win_disp *w)
{
    if (w == NULL) return;

    gtk_widget_show_all (GTK_WIDGET (w->win_main));
}

void win_disp_update (struct win_disp *w, struct mpdisplay_mpd_status *s)
{
    if ((s == NULL) || (w == NULL)) return;
    struct mpdisplay_mpd_status *cs = w->mpd_st_current;

    if (w->done) return;

    /* play/pause/stop icon */
    if ((cs == NULL) || (cs->play != s->play) || (cs->pause != s->pause)) {
        if (s->play) {
            gtk_image_set_from_icon_name (GTK_IMAGE (w->im_state), "media-playback-start-symbolic", GTK_ICON_SIZE_DIALOG);
        } else if (s->pause) {
            gtk_image_set_from_icon_name (GTK_IMAGE (w->im_state), "media-playback-pause-symbolic", GTK_ICON_SIZE_DIALOG);
        } else {
            gtk_image_set_from_icon_name (GTK_IMAGE (w->im_state), "media-playback-stop-symbolic",  GTK_ICON_SIZE_DIALOG);
        }
    }

    /* playback time */
    if ((cs == NULL) || (cs->seconds_elapsed != s->seconds_elapsed) || (cs->seconds_total != s->seconds_total)) {
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

        gtk_progress_bar_set_fraction  (GTK_PROGRESS_BAR (w->pb_time), pb_progress);
        gtk_progress_bar_set_text      (GTK_PROGRESS_BAR (w->pb_time), st_progress->str);

        if (pb_pulse) gtk_progress_bar_pulse (GTK_PROGRESS_BAR (w->pb_time));

        g_string_free (st_progress, true);
    }


    /* playlist status */
    if ((cs == NULL) || (cs->single != s->single))   gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (w->tb_single),  s->single);
    if ((cs == NULL) || (cs->shuffle != s->shuffle)) gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (w->tb_shuffle), s->shuffle);
    if ((cs == NULL) || (cs->repeat != s->repeat))   gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (w->tb_repeat),  s->repeat);

    /* volume */
    if ((cs == NULL) || (cs->volume != s->volume)) {
        gdouble volume = 0;
        if (s->volume >= 0) {
            if (s->volume <= 100) {
                volume = (gdouble) s->volume / (gdouble) 100;
            } else {
                volume = 1;
            }
        }

        gtk_progress_bar_set_fraction (GTK_PROGRESS_BAR (w->pb_volume), volume);
    }

    /* song data */
    song_data_update (w->fr_center, s, cs);

    mpdisplay_mpd_status_free (&(w->mpd_st_current));
    w->mpd_st_current = mpdisplay_mpd_status_copy (s);
}

static void song_data_update (GtkWidget *sframe, struct mpdisplay_mpd_status *s, struct mpdisplay_mpd_status *cs)
{
    if ((sframe == NULL) || (s == NULL)) return;
    if ((cs != NULL) && (!cs->success) && mpdisplay_mpd_status_tags_equal (s, cs)) return;

    /* remove old children */
    if (!((s->success) || (cs == NULL) || (cs->success))) return;

    GtkWidget *old_child = gtk_bin_get_child (GTK_BIN (sframe));
    if (old_child != NULL) {
        gtk_container_remove (GTK_CONTAINER (sframe), GTK_WIDGET (old_child));
    }

    /* add grid */
    GtkWidget *grid = gtk_grid_new ();
    gtk_widget_set_halign (grid, GTK_ALIGN_START);
    gtk_widget_set_valign (grid, GTK_ALIGN_START);

    gtk_grid_set_row_spacing     (GTK_GRID (grid), 5);
    gtk_grid_set_column_spacing  (GTK_GRID (grid), 5);
    gtk_widget_set_margin_start  (grid, 5);
    gtk_widget_set_margin_end    (grid, 5);
    gtk_widget_set_margin_top    (grid, 5);
    gtk_widget_set_margin_bottom (grid, 5);

    if (s->success) {
        int i = 0;
        for (GList *li = s->song_data->head; li != NULL; li = li->next, i++) {
            struct mpdisplay_song_data_entry *e = (struct mpdisplay_song_data_entry *) li->data;

            GtkWidget *label_name  = gtk_label_new (e->name);
            GtkWidget *label_value = gtk_label_new (e->value);

            gtk_widget_set_halign (label_name, GTK_ALIGN_START);
            gtk_widget_set_valign (label_name, GTK_ALIGN_START);

            gtk_widget_set_halign (label_value, GTK_ALIGN_FILL);
            gtk_widget_set_valign (label_value, GTK_ALIGN_START);

            gtk_label_set_line_wrap (GTK_LABEL (label_value), true);

            gtk_grid_attach (GTK_GRID (grid), label_name,  0, i, 1, 1);
            gtk_grid_attach (GTK_GRID (grid), label_value, 1, i, 1, 1);
        }
    } else {
        if ((cs == NULL) || (cs->success)) {
            GtkWidget *label = gtk_label_new ("not connected ...");
            GtkWidget *spinner = gtk_spinner_new ();

            gtk_widget_set_halign (label, GTK_ALIGN_START);
            gtk_widget_set_valign (label, GTK_ALIGN_START);

            gtk_grid_attach (GTK_GRID (grid), spinner, 0, 0, 1, 1);
            gtk_grid_attach (GTK_GRID (grid), label, 1, 0, 1, 1);

            gtk_spinner_start (GTK_SPINNER (spinner));
        }
    }

    gtk_grid_set_column_homogeneous (GTK_GRID (grid), false);
    gtk_grid_set_row_homogeneous (GTK_GRID (grid), false);

    gtk_container_add (GTK_CONTAINER (sframe), grid);
    gtk_widget_show_all (sframe);
}

/* signal handlers */
static void sh_window_close (GtkWidget *widget, gpointer data)
{
    struct win_disp* w = (struct win_disp*) data;
    w->done = true;

    gtk_main_quit ();

    w->win_main = NULL;
}
