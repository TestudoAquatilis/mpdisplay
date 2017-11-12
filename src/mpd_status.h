#ifndef __MPD_STATUS_H__
#define __MPD_STATUS_H__

#include <stdbool.h>

struct mpd_status {
    bool play;
    bool pause;
    int  seconds_elapsed;
    int  seconds_total;
    bool single;
    bool shuffle;
    bool repeat;
    int  volume;
};

struct mpd_status *mpd_status_new  ();
void               mpd_status_free (struct mpd_status **s_p);


#endif
