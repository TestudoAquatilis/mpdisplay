#include "disp_window.h"

int main (int argc, char **argv)
{
    gtk_init (&argc, &argv);

    struct disp_window *w = disp_window_new ();

    disp_window_show (w);

    gtk_main ();

    disp_window_free (&w);

    return 0;
}
