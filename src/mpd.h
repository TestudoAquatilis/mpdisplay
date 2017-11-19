#ifndef __MPD_H__
#define __MPD_H__

#include "mpd_status.h"

#ifdef __cplusplus
extern "C" {
#endif

void                         mpdisplay_mpd_free ();

struct mpdisplay_mpd_status *mpdisplay_mpd_get_status ();

#ifdef __cplusplus
}
#endif

#endif
