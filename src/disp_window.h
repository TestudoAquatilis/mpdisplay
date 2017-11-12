#ifndef __DISP_WINDOW_H__
#define __DISP_WINDOW_H__

#include <gtk/gtk.h>
#include "mpd_status.h"

struct disp_window {
    /* main window */
    GtkWidget *win_main;
    /* important widgets */
    GtkWidget *img_play;
    GtkWidget *pbar_time;
    GtkWidget *lbar_volume;
    GtkWidget *tb_consec;
    GtkWidget *tb_shuffle;
    GtkWidget *tb_repeat;
    GtkWidget *frame_song;
};

struct disp_window *disp_window_new    ();
void                disp_window_free   (struct disp_window **w_p);
void                disp_window_show   (struct disp_window *w);
void                disp_window_update (struct disp_window *w, struct mpd_status *s);

#endif
