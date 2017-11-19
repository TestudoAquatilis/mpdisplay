#ifndef WIN_DISP_H
#define WIN_DISP_H

#include <QtWidgets>

class WinDisp: public QMainWindow {
    Q_OBJECT

    public:
        WinDisp();

    private:
        /* accessible widgets */
        QLabel       *lb_state;
        QProgressBar *pb_time;
        QProgressBar *pb_volume;
        QFrame       *fr_center;
        QFormLayout  *ly_center;
        QToolButton  *bt_single;
        QToolButton  *bt_repeat;
        QToolButton  *bt_shuffle;

        /* constructor helpers */
        QWidget *create_top_row ();
        QWidget *create_bottom_row ();
        QWidget *create_new_separator_line ();
        QWidget *create_center_frame ();

        /* update helpers */
        enum StateVal {ST_PLAY, ST_PAUSE, ST_STOP};
        void update_playback_state (StateVal s);
        void update_playlist_state (bool single, bool repeat, bool shuffle);
        void update_volume (int volume);
        void update_time (int total_ms, int current_ms);
        void clear_tags ();
        void update_tags_nocon ();
};

#endif
