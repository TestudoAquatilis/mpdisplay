#ifndef __QWIN_DISP_H__
#define __QWIN_DISP_H__

#include <QtWidgets>
#include <glib.h>

class WinDisp: public QMainWindow {
    Q_OBJECT

    public:
        WinDisp ();

    private:
        /* accessible widgets */
        QLabel       *lb_state;
        QProgressBar *pb_time;
        QProgressBar *pb_volume;
        QFrame       *fr_center;
        QFormLayout  *ly_center;
        QToolButton  *tb_single;
        QToolButton  *tb_repeat;
        QToolButton  *tb_shuffle;
        QTimer       *tm_update;

        /* status */
        struct mpdisplay_mpd_status *mpd_st_current;

        /* constructor helpers */
        QWidget *create_top_row ();
        QWidget *create_bottom_row ();
        QWidget *create_new_separator_line ();
        QWidget *create_center_frame ();
        void     create_layout ();
        void     create_update_timer ();

        /* update helpers */
        enum StateVal {ST_PLAY, ST_PAUSE, ST_STOP};
        void update_playback_state (StateVal s);
        void update_playback_state (struct mpdisplay_mpd_status *st);
        void update_playlist_state (bool single, bool repeat, bool shuffle);
        void update_playlist_state (struct mpdisplay_mpd_status *st);
        void update_volume (int volume);
        void update_volume (struct mpdisplay_mpd_status *st);
        void update_time (int total_s, int elapsed_s);
        void update_time (struct mpdisplay_mpd_status *st);
        void clear_tags ();
        void update_tags_nocon ();
        void update_tags (GList *tlist);
        void update_tags (struct mpdisplay_mpd_status *st);
        void update_mpd_status (struct mpdisplay_mpd_status *st_new);

    private slots:
        void update_mpd_status ();
};

#endif
