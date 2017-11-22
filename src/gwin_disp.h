#ifndef __GWIN_DISP_H__
#define __GWIN_DISP_H__

#include <gtk/gtk.h>
#include "mpd_status.h"

#ifdef __cplusplus
extern "C" {
#endif

struct win_disp {
    /* main window */
    GtkWidget *win_main;
    /* important widgets */
    GtkWidget *im_state;
    GtkWidget *pb_time;
    GtkWidget *pb_volume;
    GtkWidget *fr_center;
    GtkWidget *ly_center;
    GtkWidget *tb_single;
    GtkWidget *tb_shuffle;
    GtkWidget *tb_repeat;
    guint      tm_update;
    /* status */
    struct mpdisplay_mpd_status *mpd_st_current;
    /* spacings */
    int _spacing;
};

struct win_disp *win_disp_new    ();
void             win_disp_free   (struct win_disp **w_p);
void             win_disp_show   (struct win_disp *w);

#ifdef __cplusplus
}
#endif

#endif
