#ifndef __MPD_STATUS_H__
#define __MPD_STATUS_H__

#include <stdbool.h>
#include <glib.h>

struct mpdisplay_song_data_entry {
    const char *name;
    const char *value;
};

struct mpdisplay_mpd_status {
    bool success;
    /* player data */
    bool play;
    bool pause;
    int  seconds_elapsed;
    int  seconds_total;
    bool single;
    bool shuffle;
    bool repeat;
    int  volume;
    /* song data */
    GStringChunk *song_data_strings;
    GQueue       *song_data;
};

struct mpdisplay_song_data_entry *mpdisplay_song_data_entry_new  ();
void                              mpdisplay_song_data_entry_free (struct mpdisplay_song_data_entry **e_p);

struct mpdisplay_mpd_status *mpdisplay_mpd_status_new  ();
struct mpdisplay_mpd_status *mpdisplay_mpd_status_copy (struct mpdisplay_mpd_status *s);
void                         mpdisplay_mpd_status_free (struct mpdisplay_mpd_status **s_p);

void                         mpdisplay_mpd_status_add_song_data       (struct mpdisplay_mpd_status *s, const char *name, const char *value);
void                         mpdisplay_mpd_status_add_song_data_entry (struct mpdisplay_mpd_status *s, const struct mpdisplay_song_data_entry *e);
bool                         mpdisplay_mpd_status_tags_equal          (struct mpdisplay_mpd_status *s1, struct mpdisplay_mpd_status *s2);


#endif
