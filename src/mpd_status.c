#include <glib.h>
#include <string.h>

#include "mpd_status.h"

struct mpdisplay_song_data_entry *mpdisplay_song_data_entry_new ()
{
    struct mpdisplay_song_data_entry *e = g_slice_new (struct mpdisplay_song_data_entry);
    if (e == NULL) return NULL;

    e->name  = NULL;
    e->value = NULL;

    return e;
}

void mpdisplay_song_data_entry_free (struct mpdisplay_song_data_entry **e_p)
{
    if (e_p == NULL) return;

    struct mpdisplay_song_data_entry *e = *e_p;

    if (e == NULL) return;

    g_slice_free (struct mpdisplay_song_data_entry, e);

    *e_p = NULL;
}

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
        struct mpdisplay_song_data_entry *e = (struct mpdisplay_song_data_entry *) li->data;

        mpdisplay_mpd_status_add_song_data_entry (sc, e);
    }

    return sc;
}

void mpdisplay_mpd_status_free (struct mpdisplay_mpd_status **s_p)
{
    if (s_p == NULL) return;

    struct mpdisplay_mpd_status *s = *s_p;

    if (s == NULL) return;

    for (GList *li = s->song_data->head; li != NULL; li = li->next) {
        struct mpdisplay_song_data_entry *e = (struct mpdisplay_song_data_entry *) li->data;
        mpdisplay_song_data_entry_free (&e);
    }
    g_queue_free (s->song_data);
    g_string_chunk_free (s->song_data_strings);

    g_slice_free (struct mpdisplay_mpd_status, s);

    *s_p = NULL;
}

void mpdisplay_mpd_status_add_song_data (struct mpdisplay_mpd_status *s, const char *name, const char *value)
{
    if (s == NULL) return;
    if (name == NULL) return;
    if (value == NULL) return;

    struct mpdisplay_song_data_entry *e = mpdisplay_song_data_entry_new ();
    e->name  = g_string_chunk_insert (s->song_data_strings, name);
    e->value = g_string_chunk_insert (s->song_data_strings, value);

    g_queue_push_tail (s->song_data, e);
}

void mpdisplay_mpd_status_add_song_data_entry (struct mpdisplay_mpd_status *s, const struct mpdisplay_song_data_entry *e)
{
    if (s == NULL) return;
    if (e == NULL) return;

    struct mpdisplay_song_data_entry *el = mpdisplay_song_data_entry_new ();
    el->name  = g_string_chunk_insert (s->song_data_strings, e->name);
    el->value = g_string_chunk_insert (s->song_data_strings, e->value);

    g_queue_push_tail (s->song_data, el);
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

        struct mpdisplay_song_data_entry *e1 = (struct mpdisplay_song_data_entry *) l1->data;
        struct mpdisplay_song_data_entry *e2 = (struct mpdisplay_song_data_entry *) l2->data;

        if (strcmp (e1->name,  e2->name)  != 0) return false;
        if (strcmp (e1->value, e2->value) != 0) return false;

        l1 = l1->next;
        l2 = l2->next;
    }

    return true;
}
