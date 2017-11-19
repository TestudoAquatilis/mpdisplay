#include <glib.h>
#include <gtk/gtk.h>
#include <stdbool.h>

#include "gwin_disp.h"
#include "options.h"
#include "mpd.h"

enum win_disp_state_val {ST_PLAY, ST_PAUSE, ST_STOP};

/* constructor helpers */
static GtkWidget *win_disp_create_top_row            (struct win_disp *w);
static GtkWidget *win_disp_create_bottom_row         (struct win_disp *w);
static GtkWidget *win_disp_create_new_separator_line (struct win_disp *w);
static GtkWidget *win_disp_create_center_frame       (struct win_disp *w);
static void       win_disp_create_layout             (struct win_disp *w);
static void       win_disp_create_update_timer       (struct win_disp *w);

/* update helpers */
static void win_disp_update_playback_state_val (struct win_disp *w, enum win_disp_state_val s);
static void win_disp_update_playback_state_st  (struct win_disp *w, struct mpdisplay_mpd_status *st);

static void win_disp_update_mpd_status_st (struct win_disp *w, struct mpdisplay_mpd_status *s);
static void win_disp_update_tags (GtkWidget *widget, struct mpdisplay_mpd_status *s, struct mpdisplay_mpd_status *cs);

/* signal handler declaration */
static void win_disp_close (GtkWidget *widget, gpointer data);
static gboolean win_disp_update_mpd_status (gpointer data_p);


/* initialization/finalization */
struct win_disp *win_disp_new ()
{
    struct win_disp *w = g_slice_new (struct win_disp);
    if (w == NULL) return NULL;

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
    w->tm_update  = -1;

    win_disp_create_layout (w);

    /* events */
    win_disp_create_update_timer (w);
    g_signal_connect (G_OBJECT (w->win_main), "destroy", G_CALLBACK (win_disp_close), (gpointer) w);

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

/*****************************************************************************************/
/* constructor helpers */
/*****************************************************************************************/
static GtkWidget *win_disp_create_top_row (struct win_disp *w)
{
    GtkWidget *result = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 1);

    /* icon */
    w->im_state = gtk_image_new ();
    win_disp_update_playback_state_val (w, ST_STOP);

    /* time bar */
    w->pb_time = gtk_progress_bar_new ();

    gtk_progress_bar_set_fraction  (GTK_PROGRESS_BAR (w->pb_time), 0.0);
    gtk_progress_bar_set_text      (GTK_PROGRESS_BAR (w->pb_time), "?");
    gtk_progress_bar_set_show_text (GTK_PROGRESS_BAR (w->pb_time), true);

    /* put everything together */
    gtk_box_pack_start (GTK_BOX (result), w->im_state, false, true, 0);
    gtk_box_pack_start (GTK_BOX (result), w->pb_time,  true,  true, 0);

    return result;
}

static GtkWidget *win_disp_create_bottom_row (struct win_disp *w)
{
    GtkWidget *result = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 1);

    /* tool buttons */
    w->tb_single  = gtk_toggle_button_new ();
    w->tb_shuffle = gtk_toggle_button_new ();
    w->tb_repeat  = gtk_toggle_button_new ();

    GtkWidget *lbl_single  = gtk_label_new ("1");
    GtkWidget *img_shuffle = gtk_image_new_from_icon_name ("media-playlist-shuffle-symbolic", GTK_ICON_SIZE_SMALL_TOOLBAR);
    GtkWidget *img_repeat  = gtk_image_new_from_icon_name ("media-playlist-repeat-symbolic", GTK_ICON_SIZE_SMALL_TOOLBAR);

    gtk_container_add (GTK_CONTAINER (w->tb_single),  lbl_single);
    gtk_container_add (GTK_CONTAINER (w->tb_shuffle), img_shuffle);
    gtk_container_add (GTK_CONTAINER (w->tb_repeat),  img_repeat);

    /* volume icon */
    GtkWidget *img_volume  = gtk_image_new_from_icon_name ("multimedia-volume-control-symbolic", GTK_ICON_SIZE_SMALL_TOOLBAR);

    /* volume bar */
    w->pb_volume  = gtk_progress_bar_new ();
    gtk_progress_bar_set_fraction  (GTK_PROGRESS_BAR (w->pb_volume), 0.0);

    /* put everything together */
    gtk_box_pack_start (GTK_BOX (result), w->tb_single,  false, true, 0);
    gtk_box_pack_start (GTK_BOX (result), w->tb_shuffle, false, true, 0);
    gtk_box_pack_start (GTK_BOX (result), w->tb_repeat,  false, true, 0);
    gtk_box_pack_end   (GTK_BOX (result), w->pb_volume,  false, true, 0);
    gtk_box_pack_end   (GTK_BOX (result), img_volume,    false, true, 0);

    return result;
}

static GtkWidget *win_disp_create_new_separator_line (struct win_disp *w)
{
    GtkWidget *result = gtk_separator_new (GTK_ORIENTATION_HORIZONTAL);
    return result;
}

static GtkWidget *win_disp_create_center_frame (struct win_disp *w)
{
    w->fr_center  = gtk_frame_new (NULL);

    gtk_frame_set_shadow_type (GTK_FRAME (w->fr_center), GTK_SHADOW_NONE);

    /* TODO: update */

    return w->fr_center;
}

static void win_disp_create_layout (struct win_disp *w)
{
    /* main layout elements */
    GtkWidget *top_row   = win_disp_create_top_row (w);
    GtkWidget *top_line  = win_disp_create_new_separator_line (w);
    GtkWidget *cen_frame = win_disp_create_center_frame (w);
    GtkWidget *bot_line  = win_disp_create_new_separator_line (w);
    GtkWidget *bot_row   = win_disp_create_bottom_row (w);

    /* arrange */
    GtkWidget *main_widget = gtk_box_new (GTK_ORIENTATION_VERTICAL, 1);
    gtk_box_pack_start (GTK_BOX (main_widget), top_row,   false, true, 0);
    gtk_box_pack_start (GTK_BOX (main_widget), top_line,  false, true, 0);
    gtk_box_pack_start (GTK_BOX (main_widget), cen_frame, true,  true, 0);
    gtk_box_pack_start (GTK_BOX (main_widget), bot_line,  false, true, 0);
    gtk_box_pack_start (GTK_BOX (main_widget), bot_row,   false, true, 0);

    /* insert */
    w->win_main = gtk_window_new (GTK_WINDOW_TOPLEVEL);
    gtk_container_add (GTK_CONTAINER (w->win_main), main_widget);

    /* title */
    gtk_window_set_title (GTK_WINDOW (w->win_main), "MPDisplay");

    /* size */
    if ((mpdisplay_options.win_width > 0) | (mpdisplay_options.win_height > 0)) {
        int w_w = mpdisplay_options.win_width;
        int w_h = mpdisplay_options.win_height;
        gtk_window_set_default_size (GTK_WINDOW (w->win_main), w_w, w_h);
    }
    if (mpdisplay_options.win_fullscreen) {
        gtk_window_fullscreen (GTK_WINDOW (w->win_main));
    }
}

static void win_disp_create_update_timer (struct win_disp *w)
{
    w->tm_update = g_timeout_add (mpdisplay_options.update_interval, win_disp_update_mpd_status, (gpointer) w);
}


/*****************************************************************************************/
/* update helpers */
/*****************************************************************************************/
static void win_disp_update_playback_state_val (struct win_disp *w, enum win_disp_state_val s)
{
    const char *st_string_stop  = "media-playback-stop-symbolic";
    const char *st_string_play  = "media-playback-start-symbolic";
    const char *st_string_pause = "media-playback-pause-symbolic";

    const char *st_string = st_string_stop;

    if (s == ST_PLAY) {
        st_string = st_string_play;
    } else if (s == ST_PAUSE) {
        st_string = st_string_pause;
    }

    gtk_image_set_from_icon_name (GTK_IMAGE (w->im_state), st_string, GTK_ICON_SIZE_DIALOG);
}

static void win_disp_update_playback_state_st (struct win_disp *w, struct mpdisplay_mpd_status *st)
{
    if ((st == NULL) || (!st->success)) {
        win_disp_update_playback_state_val (w, ST_STOP);
        return;
    }

    struct mpdisplay_mpd_status *cst = w->mpd_st_current;

    if ((cst != NULL) && (cst->success)) {
        if ((cst->play == st->play) && (cst->pause == st->pause)) return;
    }

    if (st->play) {
        win_disp_update_playback_state_val (w, ST_PLAY);
    } else if (st->pause) {
        win_disp_update_playback_state_val (w, ST_PAUSE);
    } else {
        win_disp_update_playback_state_val (w, ST_STOP);
    }
}

static gboolean win_disp_update_mpd_status (gpointer data_p)
{
    struct win_disp *w = (struct win_disp *) data_p;

    if (w == NULL) return false;

#ifdef DEBUG_NOMPD
    /* generate dummy data */
    struct mpdisplay_mpd_status *st = mpdisplay_mpd_status_new ();

    st->success = true;
    mpdisplay_mpd_status_add_song_data (st, "tag1", "value1", 0);
    mpdisplay_mpd_status_add_song_data (st, "tag2", "value2 with more content", -1);
    mpdisplay_mpd_status_add_song_data (st, "tag3", "value3 with more so much content that it should be necessary to wrap it somewhere", 0);
#else
    /* get status */
    struct mpdisplay_mpd_status *st = mpdisplay_mpd_get_status ();
#endif

    win_disp_update_mpd_status_st (w, st);

    mpdisplay_mpd_status_free (&st);

    return true;
}

static void win_disp_update_mpd_status_st (struct win_disp *w, struct mpdisplay_mpd_status *s)
{
    if ((s == NULL) || (w == NULL)) return;
    struct mpdisplay_mpd_status *cs = w->mpd_st_current;

    /* play/pause/stop icon */
    win_disp_update_playback_state_st (w, s);

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
    win_disp_update_tags (w->fr_center, s, cs);

    mpdisplay_mpd_status_free (&(w->mpd_st_current));
    w->mpd_st_current = mpdisplay_mpd_status_copy (s);
}

static void win_disp_update_tags (GtkWidget *sframe, struct mpdisplay_mpd_status *s, struct mpdisplay_mpd_status *cs)
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
static void win_disp_close (GtkWidget *widget, gpointer data)
{
    struct win_disp* w = (struct win_disp*) data;
    if (w == NULL) return;

    if (w->tm_update > 0) g_source_remove (w->tm_update);

    gtk_main_quit ();

    w->win_main = NULL;
}
