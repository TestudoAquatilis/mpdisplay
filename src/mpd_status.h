#ifndef __MPD_STATUS_H__
#define __MPD_STATUS_H__

#include <stdbool.h>

struct mpd_status {
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

struct mpd_status *mpd_status_new  ();
void               mpd_status_free (struct mpd_status **s_p);

void               mpd_status_add_song_data (struct mpd_status *s, const char *tag, const char *value);


#endif
