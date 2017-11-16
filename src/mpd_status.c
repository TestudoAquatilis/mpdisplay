#include <glib.h>
#include <string.h>

#include "mpd_status.h"

struct mpdisplay_mpd_status *mpdisplay_mpd_status_new ()
{
    struct mpdisplay_mpd_status *s = g_slice_new (struct mpdisplay_mpd_status);
    if (s == NULL) return NULL;

    s->success         = false;

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

struct mpdisplay_mpd_status *mpdisplay_mpd_status_copy (struct mpdisplay_mpd_status *s)
{
    if (s == NULL) return NULL;

    struct mpdisplay_mpd_status *sc = g_slice_new (struct mpdisplay_mpd_status);
    if (sc == NULL) return NULL;

    sc->success         = s->success;

    sc->play            = s->play;
    sc->pause           = s->pause;
    sc->seconds_elapsed = s->seconds_elapsed;
    sc->seconds_total   = s->seconds_total;
    sc->single          = s->single;
    sc->shuffle         = s->shuffle;
    sc->repeat          = s->repeat;
    sc->volume          = s->volume;

    sc->song_data_strings = g_string_chunk_new (128);
    sc->song_data         = g_queue_new ();

    for (GList *li = s->song_data->head; li != NULL; li = li->next) {
        const char *text = (const char *) li->data;
        char *local_text = g_string_chunk_insert (sc->song_data_strings, text);
        g_queue_push_tail (sc->song_data, local_text);
    }

    return sc;
}

void mpdisplay_mpd_status_free (struct mpdisplay_mpd_status **s_p)
{
    if (s_p == NULL) return;

    struct mpdisplay_mpd_status *s = *s_p;

    if (s == NULL) return;

    g_queue_free (s->song_data);
    g_string_chunk_free (s->song_data_strings);

    g_slice_free (struct mpdisplay_mpd_status, s);

    *s_p = NULL;
}

void mpdisplay_mpd_status_add_song_data (struct mpdisplay_mpd_status *s, const char *tag, const char *value)
{
    if (s == NULL) return;
    if (tag == NULL) return;
    if (value == NULL) return;

    char *local_tag   = g_string_chunk_insert (s->song_data_strings, tag);
    char *local_value = g_string_chunk_insert (s->song_data_strings, value);

    g_queue_push_tail (s->song_data, local_tag);
    g_queue_push_tail (s->song_data, local_value);
}

bool mpdisplay_mpd_status_tags_equal (struct mpdisplay_mpd_status *s1, struct mpdisplay_mpd_status *s2)
{
    if ((s1 == NULL) || (s2 == NULL)) {
        if ((s1 == NULL) && (s2 == NULL)) return true;
        return false;
    }

    GList *l1 = s1->song_data->head;
    GList *l2 = s2->song_data->head;

    while ((l1 != NULL) || (l2 != NULL)) {
        if ((l1 == NULL) || (l2 == NULL)) return false;

        char *t1 = (char *) l1->data;
        char *t2 = (char *) l2->data;

        if (strcmp (t1, t2) != 0) return false;

        l1 = l1->next;
        l2 = l2->next;
    }

    return true;
}
