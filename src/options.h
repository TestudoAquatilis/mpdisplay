#ifndef __OPTIONS_H__
#define __OPTIONS_H__

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

struct _mpdisplay_options {
    const char  *config_file;

    const char  *mpd_hostname;
    const char  *mpd_password;
    unsigned int mpd_port;
    unsigned int mpd_maxtries;

    int          win_width;
    int          win_height;
    bool         win_fullscreen;

    int          icon_size_playback;
    int          icon_size_toolbar;

    int          update_interval;

    const char  *progname;

    bool         verbose;
    bool         debug;
};

extern struct _mpdisplay_options mpdisplay_options;

bool mpdisplay_parse_options (int *argc, char ***argv);

#ifdef __cplusplus
}
#endif

#endif
