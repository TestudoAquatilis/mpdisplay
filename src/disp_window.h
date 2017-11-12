#ifndef __DISP_WINDOW_H__
#define __DISP_WINDOW_H__

#include <gtk/gtk.h>

struct disp_window {
    GtkWidget *window;
};

struct disp_window *disp_window_new  ();
void               disp_window_free (struct disp_window **win);
void               disp_window_show (struct disp_window *win);

#endif
