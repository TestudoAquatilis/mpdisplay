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

    s->song_data_strings = g_string_chunk_new (128);
    s->song_data         = g_queue_new ();

    return s;
}

void mpd_status_free (struct mpd_status **s_p)
{
    if (s_p == NULL) return;

    struct mpd_status *s = *s_p;

    g_queue_free (s->song_data);
    g_string_chunk_free (s->song_data_strings);

    g_slice_free (struct mpd_status, s);

    *s_p = NULL;
}

void mpd_status_add_song_data (struct mpd_status *s, const char *tag, const char *value)
{
    if (s == NULL) return;
    if (tag == NULL) return;
    if (value == NULL) return;

    char *local_tag   = g_string_chunk_insert (s->song_data_strings, tag);
    char *local_value = g_string_chunk_insert (s->song_data_strings, value);

    g_queue_push_tail (s->song_data, local_tag);
    g_queue_push_tail (s->song_data, local_value);
}

