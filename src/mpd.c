#include "mpd.h"
#include "options.h"
#include "mpd_status.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <mpd/client.h>

/* mpd connection */
static struct mpd_connection *connection = NULL;

/* check whether connection is working - try (re)connecting if not */
static bool mpdisplay_connection_check ()
{
    /* check state and clear existing errors */
    if (connection != NULL) {
        if (mpd_connection_get_error (connection) == MPD_ERROR_CLOSED) {
            mpdisplay_mpd_free ();
        } else if (mpd_connection_get_error (connection) != MPD_ERROR_SUCCESS) {
            mpd_connection_clear_error (connection);
        } else {
            return true;
        }
    }

    if (mpdisplay_options.debug) {
        printf ("INFO: trying to connect to %s:%d\n", mpdisplay_options.mpd_hostname, mpdisplay_options.mpd_port);
    }

    /* connect */
    connection = mpd_connection_new (mpdisplay_options.mpd_hostname, mpdisplay_options.mpd_port, 5000);
    if (connection == NULL) return false;

    if (mpd_connection_get_error (connection) != MPD_ERROR_SUCCESS) {
        fprintf (stderr, "ERROR: mpd connection failed: %s\n", mpd_connection_get_error_message (connection));
        mpd_connection_free (connection);
        connection = NULL;
        return false;
    }

    /* password */
    if (mpdisplay_options.mpd_password != NULL) {
        if (mpdisplay_options.debug) {
            printf ("INFO: sendung password: %s\n", mpdisplay_options.mpd_password);
        }
        if (! mpd_run_password (connection, mpdisplay_options.mpd_password)) {
            fprintf (stderr, "ERROR: password failed\n");
            return false;
        }
    }

    if (mpdisplay_options.debug) {
        printf ("INFO: connection to mpd established successfully\n");
    }

    return true;
}

/* parse tags to status */
static void mpdisplay_song_tags_to_status (struct mpd_song *song, struct mpdisplay_mpd_status *st)
{
    const char *albumartist = mpd_song_get_tag (song, MPD_TAG_ALBUM_ARTIST, 0);
    const char *album       = mpd_song_get_tag (song, MPD_TAG_ALBUM, 0);
    const char *track       = mpd_song_get_tag (song, MPD_TAG_TRACK, 0);
    const char *name        = mpd_song_get_tag (song, MPD_TAG_NAME, 0);
    const char *date        = mpd_song_get_tag (song, MPD_TAG_DATE, 0);

    GString *tag = g_string_new (NULL);

    if (albumartist != NULL) {
        mpdisplay_mpd_status_add_song_data (st, "Artist:", albumartist);
    }
    if (album != NULL) {
        g_string_printf (tag, "%s", album);
        if (date != NULL) {
            g_string_append_printf (tag, " (%s)", date);
        }

        mpdisplay_mpd_status_add_song_data (st, "Album:", tag->str);
    }

    /* artist */
    int i_artist = 0;
    while (true) {
        const char *artist = mpd_song_get_tag (song, MPD_TAG_ARTIST, i_artist);
        if (artist == NULL) break;

        if (albumartist != NULL) {
            /* ignore albumartist */
            if (strcmp (artist, albumartist) == 0) continue;
        }

        if (i_artist == 0) {
            g_string_printf (tag, "%s", artist);
        } else {
            g_string_append_printf (tag, ", %s", artist);
        }

        i_artist++;
    }

    if (i_artist > 0) {
        if (albumartist == NULL) {
            mpdisplay_mpd_status_add_song_data (st, "Artist:", tag->str);
        } else {
            mpdisplay_mpd_status_add_song_data (st, "Further artists:", tag->str);
        }
    }

    g_string_free (tag, true);
}

/* free connection struct */
void mpdisplay_mpd_free () {
    if (connection != NULL) {
        mpd_connection_free (connection);
        connection = NULL;
    }
}

/* return current playback status */
struct mpdisplay_mpd_status *mpdisplay_mpd_get_status ()
{
    struct mpdisplay_mpd_status *result = mpdisplay_mpd_status_new ();

    bool success = false;
    int  tries   = 0;

    while ((!success) && (tries < mpdisplay_options.mpd_maxtries)) {
        tries++;

        if (! mpdisplay_connection_check ()) continue;

        /* get status */
        struct mpd_status *status = NULL;

        status = mpd_run_status (connection);

        if (status == NULL) {
            if (mpd_connection_get_error (connection) != MPD_ERROR_SUCCESS) {
                fprintf (stderr, "ERROR obtaining mpd status: %s\n", mpd_connection_get_error_message (connection));
            } else {
                fprintf (stderr, "ERROR obtaining mpd status:\n");
            }
            continue;
        }

        /* get song */
        struct mpd_song *song = NULL;

        song = mpd_run_current_song (connection);

        if (song == NULL) {
            if (mpd_connection_get_error (connection) != MPD_ERROR_SUCCESS) {
                fprintf (stderr, "ERROR obtaining mpd status: %s\n", mpd_connection_get_error_message (connection));
                continue;
            }
            /* else: no current song */
        }

        /* parse status */
        success = true;

        /* play/pause */
        result->play  = (mpd_status_get_state (status) == MPD_STATE_PLAY  ? true : false);
        result->pause = (mpd_status_get_state (status) == MPD_STATE_PAUSE ? true : false);

        /* time */
        result->seconds_elapsed = mpd_status_get_elapsed_time (status);
        result->seconds_total   = mpd_status_get_total_time (status);

        /* single/shuffle/repeat */
        result->single  = mpd_status_get_single (status);
        result->shuffle = mpd_status_get_random (status);
        result->repeat  = mpd_status_get_repeat (status);

        /* volume */
        result->volume = mpd_status_get_volume (status);

        /* song data */
        if (song != NULL) {
            mpdisplay_song_tags_to_status (song, result);
        }

        if (status != NULL) {
            mpd_status_free (status);
        }
        if (song != NULL) {
            mpd_song_free (song);
        }
    }

    if (success) result->success = true;

    return result;
}

