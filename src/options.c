#include "options.h"

#include <glib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct _mpdisplay_options mpdisplay_options = {
    .config_file        = NULL,
    .mpd_hostname       = "localhost",
    .mpd_password       = NULL,
    .mpd_port           = 6600,
    .mpd_maxtries       = 2,
    .win_width          = -1,
    .win_height         = -1,
    .win_fullscreen     = false,
    .icon_size_toolbar  = -1,
    .icon_size_playback = -1,
    .update_interval    = 500,
    .progname           = "mpdisplay",
    .verbose            = false,
    .debug              = false
};

struct option_file_data {
    const gchar *group_name;
    const gchar *key_name;
    GOptionArg   arg;
    gpointer     arg_data;
};

static GOptionEntry option_entries [] = {
    {"config",        'c', 0, G_OPTION_ARG_FILENAME, &(mpdisplay_options.config_file),    "Configuration file",                                "filename"},
    {"hostname",      'H', 0, G_OPTION_ARG_STRING,   &(mpdisplay_options.mpd_hostname),   "Hostname of host running mpd - default: localhost", "host"},
    {"password",      'p', 0, G_OPTION_ARG_STRING,   &(mpdisplay_options.mpd_password),   "Password of mpd",                                   "password"},
    {"port",          'P', 0, G_OPTION_ARG_INT,      &(mpdisplay_options.mpd_port),       "Port of mpd - default: port=6600",                  "port"},
    {"maxtries",      'm', 0, G_OPTION_ARG_INT,      &(mpdisplay_options.mpd_maxtries),   "Maximum tries for sending mpd commands",            "n"},
    {"window-width",  'w', 0, G_OPTION_ARG_INT,      &(mpdisplay_options.win_width),      "Set default width of window",                       "n"},
    {"window-height", 'h', 0, G_OPTION_ARG_INT,      &(mpdisplay_options.win_height),     "Set default height of window",                      "n"},
    {"fullscreen",    'F', 0, G_OPTION_ARG_NONE,     &(mpdisplay_options.win_fullscreen), "Start in fullscreen mode",                          NULL},
    {"update-ms",     'u', 0, G_OPTION_ARG_INT,      &(mpdisplay_options.update_interval),"Set update interval for mpd status to <t> ms",      "t"},
    {"verbose",       'v', 0, G_OPTION_ARG_NONE,     &(mpdisplay_options.verbose),        "Set to verbose",                                    NULL},
    {"debug",         'd', 0, G_OPTION_ARG_NONE,     &(mpdisplay_options.debug),          "Activate debug output",                             NULL},
    {NULL}
};

static struct option_file_data cfg_file_entries [] = {
    {"mpd",     "hostname",      G_OPTION_ARG_STRING,   &(mpdisplay_options.mpd_hostname)},
    {"mpd",     "password",      G_OPTION_ARG_STRING,   &(mpdisplay_options.mpd_password)},
    {"mpd",     "port",          G_OPTION_ARG_INT,      &(mpdisplay_options.mpd_port)},
    {"mpd",     "maxtries",      G_OPTION_ARG_INT,      &(mpdisplay_options.mpd_maxtries)},
    {"window",  "width",         G_OPTION_ARG_INT,      &(mpdisplay_options.win_width)},
    {"window",  "height",        G_OPTION_ARG_INT,      &(mpdisplay_options.win_height)},
    {"window",  "fullscreen",    G_OPTION_ARG_NONE,     &(mpdisplay_options.win_fullscreen)},
    {"control", "update-ms",     G_OPTION_ARG_INT,      &(mpdisplay_options.update_interval)},
    {"icon",    "size-toolbar",  G_OPTION_ARG_INT,      &(mpdisplay_options.icon_size_toolbar)},
    {"icon",    "size-playback", G_OPTION_ARG_INT,      &(mpdisplay_options.icon_size_playback)},
    {NULL}
};

static bool mpdisplay_options_from_file ()
{
    GError   *error    = NULL;
    GKeyFile *key_file = g_key_file_new ();

    if (!g_key_file_load_from_file (key_file, mpdisplay_options.config_file, G_KEY_FILE_NONE, &error)) {
        fprintf (stderr, "Error parsing config file: %s\n", error->message);
        g_error_free (error);
        return false;
    }
    
    /* try finding options */
    for (struct option_file_data *entry = &(cfg_file_entries[0]); entry->group_name != NULL; entry++) {
        g_clear_error (&error);
        if (entry->arg == G_OPTION_ARG_NONE) {
            gboolean tempbool;
            tempbool = g_key_file_get_boolean (key_file, entry->group_name, entry->key_name, &error);
            if (error == NULL) {
                bool *targetbool = (bool *) entry->arg_data;
                *targetbool = tempbool;
            } else {
                if ((error->code != G_KEY_FILE_ERROR_GROUP_NOT_FOUND) && (error->code != G_KEY_FILE_ERROR_KEY_NOT_FOUND)) {
                    g_key_file_free (key_file);
                    fprintf (stderr, "Error parsing config file %s: %s\n", mpdisplay_options.config_file, error->message);
                    g_error_free (error);
                    return false;
                }
            }
        } else if (entry->arg == G_OPTION_ARG_INT) {
            gint tempint;
            tempint = g_key_file_get_integer (key_file, entry->group_name, entry->key_name, &error);
            if (error == NULL) {
                int *targetint = (int *) entry->arg_data;
                *targetint = tempint;
            } else {
                if ((error->code != G_KEY_FILE_ERROR_GROUP_NOT_FOUND) && (error->code != G_KEY_FILE_ERROR_KEY_NOT_FOUND)) {
                    g_key_file_free (key_file);
                    fprintf (stderr, "Error parsing config file %s: %s\n", mpdisplay_options.config_file, error->message);
                    g_error_free (error);
                    return false;
                }
            }
        } else if ((entry->arg == G_OPTION_ARG_STRING) || (entry->arg == G_OPTION_ARG_FILENAME)) {
            gchar *tempstr;
            tempstr = g_key_file_get_string (key_file, entry->group_name, entry->key_name, &error);
            if (mpdisplay_options.debug) {
                printf ("INFO: parsed string %s for option %s\n", tempstr, entry->key_name);
            }
            if (tempstr != NULL) {
                gchar **targetstr = (gchar **) entry->arg_data;
                *targetstr = tempstr;
            }
        }
    }

    /* free */
    g_key_file_free (key_file);
    if (error != NULL) {
        g_error_free (error);
    }

    return true;
}

/* option checking */
static bool options_check ()
{
    if (mpdisplay_options.config_file != NULL) {
        if (mpdisplay_options.debug) {
            printf ("config-file: %s\n", mpdisplay_options.config_file);
        }
    }
    if (mpdisplay_options.mpd_hostname == NULL) {
        fprintf (stderr, "ERROR: no hostname specified\n");
        return false;
    } else {
        if (mpdisplay_options.debug) {
            printf ("hostname: %s\n", mpdisplay_options.mpd_hostname);
        }
    }
    if (mpdisplay_options.mpd_port >= (1 << 16)) {
        fprintf (stderr, "ERROR: port needs to be in range 0 ... %d\n", (1 << 16));
        return false;
    } else {
        if (mpdisplay_options.debug) {
            printf ("port: %d\n", mpdisplay_options.mpd_port);
        }
    }
    if (mpdisplay_options.mpd_password != NULL) {
        if (mpdisplay_options.debug) {
            printf ("password: %s\n", mpdisplay_options.mpd_password);
        }
    }
    if (mpdisplay_options.debug) {
        printf ("mpd-maxtries: %d\n", mpdisplay_options.mpd_maxtries);
    }
    if (mpdisplay_options.update_interval <= 0) {
        fprintf (stderr, "ERROR: update interval must be > 0 (was %d)\n", mpdisplay_options.update_interval);
        return false;
    } else {
        if (mpdisplay_options.debug) {
            printf ("update interval: %d ms\n", mpdisplay_options.update_interval);
        }
    }

    return true;
}


bool mpdisplay_parse_options (int *argc, char ***argv)
{
    /* option parsing */
    GError         *error          = NULL;
    GOptionContext *option_context = g_option_context_new ("show mpd playback status for small displays");

    g_option_context_add_main_entries (option_context, option_entries, NULL);

    if (!g_option_context_parse (option_context, argc, argv, &error)) {
        fprintf (stderr, "Error parsing options: %s\n", error->message);
        goto mpdisplay_options_exit_error;
    }

    if (mpdisplay_options.config_file != NULL) {
        if (!mpdisplay_options_from_file ()) {
            goto mpdisplay_options_exit_error;
        }
    }

    if (!options_check ()) {
        goto mpdisplay_options_exit_error;
    }

    g_option_context_free (option_context);

    if (error != NULL) {
        g_error_free (error);
    }

    return true;

mpdisplay_options_exit_error:
    g_option_context_free (option_context);
    if (error != NULL) {
        g_error_free (error);
    }
    return false;
}



