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
static void win_disp_update_playback_state_val  (struct win_disp *w, enum win_disp_state_val s);
static void win_disp_update_playback_state_st   (struct win_disp *w, struct mpdisplay_mpd_status *st);
static void win_disp_update_playlist_state_bool (struct win_disp *w, bool single, bool repeat, bool shuffle);
static void win_disp_update_playlist_state_st   (struct win_disp *w, struct mpdisplay_mpd_status *st);
static void win_disp_update_volume_int          (struct win_disp *w, int volume);
static void win_disp_update_volume_st           (struct win_disp *w, struct mpdisplay_mpd_status *st);
static void win_disp_update_time_int            (struct win_disp *w, int total_s, int elapsed_s);
static void win_disp_update_time_st             (struct win_disp *w, struct mpdisplay_mpd_status *st);
static void win_disp_clear_tags                 (struct win_disp *w);
static void win_disp_update_tags_nocon          (struct win_disp *w);
static void win_disp_update_tags_glist          (struct win_disp *w, GList *tlist);
static void win_disp_update_tags_st             (struct win_disp *w, struct mpdisplay_mpd_status *st);
static void win_disp_update_mpd_status_st       (struct win_disp *w, struct mpdisplay_mpd_status *st);

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
    w->tb_single  = NULL;
    w->tb_repeat  = NULL;
    w->tb_shuffle = NULL;
    w->fr_center  = NULL;
    w->ly_center  = NULL;
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
    GtkWidget *result = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, w->_spacing);

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
    GtkWidget *result = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, w->_spacing);

    /* tool buttons */
    w->tb_single  = gtk_toggle_button_new ();
    w->tb_repeat  = gtk_toggle_button_new ();
    w->tb_shuffle = gtk_toggle_button_new ();

    GtkWidget *lbl_single  = gtk_label_new ("1");
    GtkWidget *img_repeat  = gtk_image_new_from_icon_name ("media-playlist-repeat-symbolic", GTK_ICON_SIZE_SMALL_TOOLBAR);
    GtkWidget *img_shuffle = gtk_image_new_from_icon_name ("media-playlist-shuffle-symbolic", GTK_ICON_SIZE_SMALL_TOOLBAR);

    /* sizing */
    gint width;
    gint height;
    gtk_icon_size_lookup (GTK_ICON_SIZE_SMALL_TOOLBAR, &width, &height);
    gtk_widget_set_size_request (lbl_single, width, height);
    PangoAttrList *font_attr_list = pango_attr_list_new ();
    pango_attr_list_insert (font_attr_list, pango_attr_size_new_absolute ((width * 8 * PANGO_SCALE) / 10));
    gtk_label_set_attributes (GTK_LABEL(lbl_single), font_attr_list);
    pango_attr_list_unref (font_attr_list);

    gtk_container_add (GTK_CONTAINER (w->tb_single),  lbl_single);
    gtk_container_add (GTK_CONTAINER (w->tb_repeat),  img_repeat);
    gtk_container_add (GTK_CONTAINER (w->tb_shuffle), img_shuffle);

    /* volume icon */
    GtkWidget *img_volume  = gtk_image_new_from_icon_name ("multimedia-volume-control-symbolic", GTK_ICON_SIZE_SMALL_TOOLBAR);

    /* volume bar */
    w->pb_volume  = gtk_progress_bar_new ();
    gtk_progress_bar_set_fraction  (GTK_PROGRESS_BAR (w->pb_volume), 0.0);

    /* put everything together */
    gtk_box_pack_start (GTK_BOX (result), w->tb_single,  false, true, 0);
    gtk_box_pack_start (GTK_BOX (result), w->tb_repeat,  false, true, 0);
    gtk_box_pack_start (GTK_BOX (result), w->tb_shuffle, false, true, 0);
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
    w->fr_center = gtk_scrolled_window_new (NULL, NULL);

    gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (w->fr_center), GTK_SHADOW_NONE);

    gtk_scrolled_window_set_propagate_natural_width  (GTK_SCROLLED_WINDOW (w->fr_center), true);
    gtk_scrolled_window_set_propagate_natural_height (GTK_SCROLLED_WINDOW (w->fr_center), false);
    gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (w->fr_center), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);

    gtk_widget_set_halign (w->fr_center, GTK_ALIGN_FILL);

    win_disp_update_tags_nocon (w);

    return w->fr_center;
}

static void win_disp_create_layout (struct win_disp *w)
{
    w->_spacing = 10;

    /* main layout elements */
    GtkWidget *top_row   = win_disp_create_top_row (w);
    GtkWidget *top_line  = win_disp_create_new_separator_line (w);
    GtkWidget *cen_frame = win_disp_create_center_frame (w);
    GtkWidget *bot_line  = win_disp_create_new_separator_line (w);
    GtkWidget *bot_row   = win_disp_create_bottom_row (w);

    /* arrange */
    GtkWidget *main_widget = gtk_box_new (GTK_ORIENTATION_VERTICAL, w->_spacing);
    gtk_box_pack_start (GTK_BOX (main_widget), top_row,   false, true, 0);
    gtk_box_pack_start (GTK_BOX (main_widget), top_line,  false, true, 0);
    gtk_box_pack_start (GTK_BOX (main_widget), cen_frame, true,  true, 0);
    gtk_box_pack_start (GTK_BOX (main_widget), bot_line,  false, true, 0);
    gtk_box_pack_start (GTK_BOX (main_widget), bot_row,   false, true, 0);

    /* insert */
    w->win_main = gtk_window_new (GTK_WINDOW_TOPLEVEL);
    gtk_container_add (GTK_CONTAINER (w->win_main), main_widget);
    gtk_container_set_border_width (GTK_CONTAINER (w->win_main), w->_spacing);

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

static void win_disp_update_playlist_state_bool (struct win_disp *w, bool single, bool repeat, bool shuffle)
{
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (w->tb_single),  single);
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (w->tb_repeat),  repeat);
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (w->tb_shuffle), shuffle);
}

static void win_disp_update_playlist_state_st (struct win_disp *w, struct mpdisplay_mpd_status *st)
{
    if ((st == NULL) || (!st->success)) {
        win_disp_update_playlist_state_bool (w, false, false, false);
        return;
    }

    struct mpdisplay_mpd_status *cst = w->mpd_st_current;

    if ((cst != NULL) && (cst->success)) {
        if ((cst->single == st->single) && (cst->repeat == st->repeat) && (cst->shuffle == st->shuffle)) return;
    }

    win_disp_update_playlist_state_bool (w, st->single, st->repeat, st->shuffle);
}

static void win_disp_update_volume_int (struct win_disp *w, int volume)
{
    gdouble gd_volume;
    if (volume < 0) {
        gd_volume = 0.0;
    } else if (volume > 100) {
        gd_volume = 1.0;
    } else {
        gd_volume = (gdouble) volume / (gdouble) 100.0;
    }

    gtk_progress_bar_set_fraction (GTK_PROGRESS_BAR (w->pb_volume), gd_volume);
}

static void win_disp_update_volume_st (struct win_disp *w, struct mpdisplay_mpd_status *st)
{
    if ((st == NULL) || (!st->success)) {
        win_disp_update_volume_int (w, 0);
        return;
    }

    struct mpdisplay_mpd_status *cst = w->mpd_st_current;

    if ((cst != NULL) && (cst->success)) {
        if (cst->volume == st->volume) return;
    }

    win_disp_update_volume_int (w, st->volume);
}

static void win_disp_update_time_int (struct win_disp *w, int total_s, int elapsed_s)
{
    GString *st_progress = g_string_new (NULL);

    if (elapsed_s >= 0) {
        g_string_printf (st_progress, "%d:%02d", elapsed_s/60, elapsed_s%60);
        if (total_s >= 0) {
            g_string_append_printf (st_progress, " / %d:%02d", total_s/60, total_s%60);
        } else {
            total_s = elapsed_s;
        }
    } else {
        elapsed_s = 0;
        if (total_s >= 0) {
            g_string_printf (st_progress, "%d:%02d", total_s/60, total_s%60);
        } else {
            st_progress = g_string_assign (st_progress, "?");
            total_s = 0;
        }
    }

    if (total_s < 1) total_s = 1;

    gdouble pb_progress = (gdouble) elapsed_s / (gdouble) total_s;

    gtk_progress_bar_set_fraction  (GTK_PROGRESS_BAR (w->pb_time), pb_progress);
    gtk_progress_bar_set_text      (GTK_PROGRESS_BAR (w->pb_time), st_progress->str);

    g_string_free (st_progress, true);
}

static void win_disp_update_time_st (struct win_disp *w, struct mpdisplay_mpd_status *st)
{
    if ((st == NULL) || (!st->success)) {
        win_disp_update_time_int (w, 0, 0);
        return;
    }

    struct mpdisplay_mpd_status *cst = w->mpd_st_current;

    if ((cst != NULL) && (cst->success)) {
        if ((cst->seconds_elapsed == st->seconds_elapsed) && (cst->seconds_total == st->seconds_total)) return;
    }

    win_disp_update_time_int (w, st->seconds_total, st->seconds_elapsed);
}

static void win_disp_clear_tags (struct win_disp *w)
{
    GtkWidget *cen_frame = w->fr_center;

    GtkWidget *old_child = gtk_bin_get_child (GTK_BIN (cen_frame));
    if (old_child != NULL) {
        gtk_container_remove (GTK_CONTAINER (cen_frame), GTK_WIDGET (old_child));
    }

    /* add grid */
    GtkWidget *grid = gtk_grid_new ();
    gtk_widget_set_halign (grid, GTK_ALIGN_FILL);

    gtk_grid_set_row_spacing     (GTK_GRID (grid), 5);
    gtk_grid_set_column_spacing  (GTK_GRID (grid), 5);
    gtk_widget_set_margin_start  (grid, 5);
    gtk_widget_set_margin_end    (grid, 5);
    gtk_widget_set_margin_top    (grid, 5);
    gtk_widget_set_margin_bottom (grid, 5);

    gtk_grid_set_column_homogeneous (GTK_GRID (grid), false);
    gtk_grid_set_row_homogeneous    (GTK_GRID (grid), false);

    gtk_container_add (GTK_CONTAINER (cen_frame), grid);

    w->ly_center = grid;
}

static void win_disp_update_tags_nocon (struct win_disp *w)
{
    win_disp_clear_tags (w);
    GtkWidget *grid = w->ly_center;

    GtkWidget *label   = gtk_label_new ("not connected ...");
    GtkWidget *spinner = gtk_spinner_new ();

    gtk_widget_set_halign (label, GTK_ALIGN_START);
    gtk_widget_set_valign (label, GTK_ALIGN_START);

    gtk_grid_attach (GTK_GRID (grid), spinner, 0, 0, 1, 1);
    gtk_grid_attach (GTK_GRID (grid), label,   1, 0, 1, 1);

    gtk_spinner_start (GTK_SPINNER (spinner));

    gtk_widget_show_all (w->fr_center);
}

static void win_disp_update_tags_glist (struct win_disp *w, GList *tlist)
{
    win_disp_clear_tags (w);
    GtkWidget *grid = w->ly_center;

    bool do_attr = false;
    PangoAttrList *font_attr_list;

    if (mpdisplay_options.win_prioscale) {
        do_attr = true;
        font_attr_list = pango_attr_list_new ();
        pango_attr_list_insert (font_attr_list, pango_attr_scale_new (0.75));
    }


    int i = 0;
    for (GList *li = tlist; li != NULL; li = li->next, i++) {
        struct mpdisplay_song_data_entry *e = (struct mpdisplay_song_data_entry *) li->data;

        GtkWidget *label_name  = gtk_label_new (e->name);
        GtkWidget *label_value = gtk_label_new (e->value);

        gtk_widget_set_halign (label_name, GTK_ALIGN_START);
        gtk_widget_set_valign (label_name, GTK_ALIGN_START);

        gtk_widget_set_halign (label_value, GTK_ALIGN_FILL);
        gtk_widget_set_valign (label_value, GTK_ALIGN_START);

        gtk_widget_set_hexpand  (label_value, true);
        gtk_label_set_justify   (GTK_LABEL (label_value), GTK_JUSTIFY_CENTER);
        gtk_label_set_line_wrap (GTK_LABEL (label_value), true);

        if (do_attr && (e->priority < 0)) {
            gtk_label_set_attributes (GTK_LABEL(label_name),  font_attr_list);
            gtk_label_set_attributes (GTK_LABEL(label_value), font_attr_list);
        }

        gtk_grid_attach (GTK_GRID (grid), label_name,  0, i, 1, 1);
        gtk_grid_attach (GTK_GRID (grid), label_value, 1, i, 1, 1);
    }

    gtk_widget_show_all (w->fr_center);

    if (do_attr) pango_attr_list_unref (font_attr_list);
}

static void win_disp_update_tags_st (struct win_disp *w, struct mpdisplay_mpd_status *st)
{
    struct mpdisplay_mpd_status *cst = w->mpd_st_current;

    if ((st == NULL) || (!st->success)) {
        if ((cst != NULL) && (cst->success)) {
            win_disp_update_tags_nocon (w);
        }
        return;
    }

    if ((cst != NULL) && (cst->success)) {
        if (mpdisplay_mpd_status_tags_equal (cst, st)) return;
    }

    win_disp_update_tags_glist (w, st->song_data->head);
}


static void win_disp_update_mpd_status_st (struct win_disp *w, struct mpdisplay_mpd_status *st)
{
    if (w == NULL) return;

    win_disp_update_playback_state_st (w, st);
    win_disp_update_playlist_state_st (w, st);
    win_disp_update_volume_st (w, st);
    win_disp_update_time_st (w, st);
    win_disp_update_tags_st (w, st);

    mpdisplay_mpd_status_free (&(w->mpd_st_current));
    w->mpd_st_current = mpdisplay_mpd_status_copy (st);
}

/*****************************************************************************************/
/* handlers */
/*****************************************************************************************/
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

static void win_disp_close (GtkWidget *widget, gpointer data)
{
    struct win_disp* w = (struct win_disp*) data;
    if (w == NULL) return;

    if (w->tm_update > 0) g_source_remove (w->tm_update);

    gtk_main_quit ();

    w->win_main = NULL;
}
