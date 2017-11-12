#include <gtk/gtk.h>

#include "disp_window.h"
#include "options.h"
#include "mpd.h"

struct us_data {
    struct disp_window *w;
};

static gboolean update_status (gpointer data_p)
{
    struct us_data *data = (struct us_data *) data_p;

    if (data->w == NULL) return false;

    /* generate dummy data */
    struct mpdisplay_mpd_status *st = mpdisplay_mpd_get_status ();

    disp_window_update (data->w, st);

    mpdisplay_mpd_status_free (&st);

    return true;
}

int main (int argc, char **argv)
{
    gtk_init (&argc, &argv);
    if (!mpdisplay_parse_options (&argc, &argv)) return 1;

    struct disp_window *w = disp_window_new ();

    /* init status: unconnected */
    struct mpdisplay_mpd_status *st = mpdisplay_mpd_status_new ();
    disp_window_update (w, st);
    mpdisplay_mpd_status_free (&st);

    disp_window_show (w);

    struct us_data usd = {w};
    g_timeout_add (500, update_status, (gpointer) &usd);

    gtk_main ();

    usd.w = NULL;
    disp_window_free (&w);
    mpdisplay_mpd_free ();

    return 0;
}
