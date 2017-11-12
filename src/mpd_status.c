#include <glib.h>

#include "mpd_status.h"

struct mpd_status *mpd_status_new ()
{
    struct mpd_status *s = g_slice_new (struct mpd_status);
    if (s == NULL) return NULL;

    s->play            = false;
    s->pause           = false;
    s->seconds_elapsed = -1;
    s->seconds_total   = -1;
    s->single          = false;
    s->shuffle         = false;
    s->repeat          = false;
    s->volume          = -1;

    return s;
}

void mpd_status_free (struct mpd_status **s_p)
{
    if (s_p == NULL) return;

    struct mpd_status *s = *s_p;

    g_slice_free (struct mpd_status, s);

    *s_p = NULL;
}

