#ifndef __DISP_WINDOW_H__
#define __DISP_WINDOW_H__

#include <gtk/gtk.h>

struct disp_window {
    /* main window */
    GtkWidget *window;
    /* main vbox */
    GtkWidget *vbox0;
    GtkWidget *vbox0_s0;
    GtkWidget *vbox0_s1;
    /* hbox playing */
    GtkWidget *hbox1p;
    GtkWidget *img_play;
    GtkWidget *pbar_time;
    /* hbox status */
    GtkWidget *hbox1s;
};

struct disp_window *disp_window_new  ();
void                disp_window_free (struct disp_window **win);
void                disp_window_show (struct disp_window *win);

#endif
