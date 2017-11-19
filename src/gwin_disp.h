#ifndef __GWIN_DISP_H__
#define __GWIN_DISP_H__

#include <gtk/gtk.h>
#include "mpd_status.h"

#ifdef __cplusplus
extern "C" {
#endif

struct win_disp {
    bool done;
    /* main window */
    GtkWidget *win_main;
    /* important widgets */
    GtkWidget *img_play;
    GtkWidget *pbar_time;
    GtkWidget *pbar_volume;
    GtkWidget *tb_single;
    GtkWidget *tb_shuffle;
    GtkWidget *tb_repeat;
    GtkWidget *frame_song;
    /* status */
    struct mpdisplay_mpd_status *current_status;
};

struct win_disp *win_disp_new    ();
void             win_disp_free   (struct win_disp **w_p);
void             win_disp_show   (struct win_disp *w);
void             win_disp_update (struct win_disp *w, struct mpdisplay_mpd_status *s);

#ifdef __cplusplus
}
#endif

#endif
