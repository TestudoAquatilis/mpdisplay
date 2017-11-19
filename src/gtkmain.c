#include <gtk/gtk.h>

#include "gwin_disp.h"
#include "options.h"
#include "mpd.h"

int main (int argc, char **argv)
{
    gtk_init (&argc, &argv);
    if (!mpdisplay_parse_options (&argc, &argv)) return 1;

    struct win_disp *w = win_disp_new ();

    win_disp_show (w);

    gtk_main ();

    win_disp_free (&w);
    mpdisplay_mpd_free ();

    return 0;
}
