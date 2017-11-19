#include <QtWidgets>
#include <glib.h>

#include "win_disp.h"
#include "options.h"
#include "mpd.h"

WinDisp::WinDisp()
{
    mpd_st_current = NULL;
    create_layout ();
    create_update_timer ();
    tm_update->start ();
}

void WinDisp::create_layout ()
{
    QWidget *top_row   = create_top_row ();
    QWidget *top_line  = create_new_separator_line ();
    QWidget *cen_frame = create_center_frame ();
    QWidget *bot_line  = create_new_separator_line ();
    QWidget *bot_row   = create_bottom_row ();

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(top_row,   0);
    mainLayout->addWidget(top_line,  0);
    mainLayout->addWidget(cen_frame, 1);
    mainLayout->addWidget(bot_line,  0);
    mainLayout->addWidget(bot_row,   0);

    QWidget *mainWidget = new QWidget;
    mainWidget->setLayout(mainLayout);
    setCentralWidget (mainWidget);

    setWindowTitle("MPDisplay");
}

QWidget *WinDisp::create_top_row ()
{
    QWidget *result = new QWidget;

    /* icon */
    lb_state = new QLabel;
    update_playback_state (ST_STOP);

    /* time bar */
    pb_time = new QProgressBar;
    pb_time->setFormat ("0:00");
    pb_time->setTextVisible (true);

    pb_time->setMinimum (0);
    pb_time->setMaximum (60);
    pb_time->setValue (0);

    /* put everything together */
    QHBoxLayout *layout = new QHBoxLayout;
    layout->addWidget (lb_state, 0);
    layout->addWidget (pb_time,  1);

    result->setLayout (layout);

    return result;
}

QWidget *WinDisp::create_bottom_row ()
{
    QWidget *result = new QWidget;

    /* icons */
    int icon_size = QApplication::style()->pixelMetric(QStyle::PM_ToolBarIconSize);

    /* tool buttons */
    bt_single  = new QToolButton;
    bt_repeat  = new QToolButton;
    bt_shuffle = new QToolButton;

    bt_single->setText ("1");
    bt_repeat->setIcon (QIcon::fromTheme ("media-playlist-repeat-symbolic"));
    bt_shuffle->setIcon (QIcon::fromTheme ("media-playlist-shuffle-symbolic"));

    bt_single->setCheckable(true);
    bt_repeat->setCheckable(true);
    bt_shuffle->setCheckable(true);

    /* volume icon */
    QLabel *lb_volume = new QLabel;
    lb_volume->setPixmap (QIcon::fromTheme ("player-volume").pixmap (icon_size, icon_size));

    /* volume bar */
    pb_volume = new QProgressBar;
    pb_volume->setFormat ("%p\%");
    pb_volume->setTextVisible (true);

    pb_volume->setMinimum (0);
    pb_volume->setMaximum (100);
    pb_volume->setValue (0);

    /* put everything together */
    QHBoxLayout *layout = new QHBoxLayout;
    layout->addWidget (bt_single, 0);
    layout->addWidget (bt_repeat, 0);
    layout->addWidget (bt_shuffle, 0);
    layout->addStretch (1);
    layout->addWidget (lb_volume, 0);
    layout->addWidget (pb_volume, 0);

    result->setLayout (layout);

    return result;
}

QWidget *WinDisp::create_center_frame ()
{
    fr_center = new QFrame;
    ly_center = new QFormLayout;

    fr_center->setFrameShape (QFrame::NoFrame);
    fr_center->setLayout     (ly_center);

    update_tags_nocon ();

    return fr_center;
}

QWidget *WinDisp::create_new_separator_line ()
{
    QFrame *result = new QFrame;

    result->setFrameShape (QFrame::HLine);
    result->setFrameShadow (QFrame::Sunken);

    return result;
}

void WinDisp::create_update_timer ()
{
    tm_update = new QTimer (this);

    tm_update->setSingleShot (false);
    tm_update->setInterval (mpdisplay_options.update_interval);

    connect (tm_update, SIGNAL (timeout()), this, SLOT (update_mpd_status()));
}

void WinDisp::update_playback_state (StateVal s)
{
    const char *st_string_stop  = "media-playback-stop";
    const char *st_string_play  = "media-playback-start";
    const char *st_string_pause = "media-playback-pause";

    const char *st_string = st_string_stop;

    if (s == ST_PLAY) {
        st_string = st_string_play;
    } else if (s == ST_PAUSE) {
        st_string = st_string_pause;
    }

    int icon_size = QApplication::style()->pixelMetric(QStyle::PM_LargeIconSize);
    lb_state->setPixmap (QIcon::fromTheme (st_string).pixmap (icon_size, icon_size));
}

void WinDisp::update_playlist_state (bool single, bool repeat, bool shuffle)
{
    bt_single->setChecked (single);
    bt_repeat->setChecked (repeat);
    bt_shuffle->setChecked (shuffle);
}

void WinDisp::update_volume (int volume)
{
    if (volume < 0) {
        pb_volume->setValue (0);
    } else if (volume > 100) {
        pb_volume->setValue (100);
    } else {
        pb_volume->setValue (volume);
    }
}

void WinDisp::update_time (int total_s, int elapsed_s)
{
    GString *st_progress = g_string_new (NULL);

    if (elapsed_s >= 0) {
        g_string_printf (st_progress, "%d:%02d", elapsed_s/60, elapsed_s%60);
        if (total_s >= 0) {
            g_string_append_printf (st_progress, " / %d:%02d", total_s/60, total_s%60);
        } else {
            total_s = elapsed_s;
        }
    } else {
        elapsed_s = 0;
        if (total_s >= 0) {
            g_string_printf (st_progress, "%d:%02d", total_s/60, total_s%60);
        } else {
            st_progress = g_string_assign (st_progress, "?");
            total_s = 0;
        }
    }

    if (total_s < 1) total_s = 1;

    pb_time->setMaximum (total_s);
    pb_time->setValue   (elapsed_s);
    pb_time->setFormat  (st_progress->str);

    g_string_free (st_progress, true);
}

void WinDisp::clear_tags ()
{
    for (int i = ly_center->rowCount() - 1; i >= 0; i--) {
        ly_center->removeRow (i);
    }
}

void WinDisp::update_tags_nocon ()
{
    clear_tags ();

    QLabel *lbl_msg   = new QLabel ("not connected");
    QLabel *lbl_dummy = new QLabel ("...");

    ly_center->addRow (lbl_msg, lbl_dummy);
}

void WinDisp::update_mpd_status (struct mpdisplay_mpd_status *st_new)
{
    printf ("DEBUG.... mpd status update\n");

    mpdisplay_mpd_status_free (&st_new);
}

void WinDisp::update_mpd_status ()
{
#ifdef DEBUG_NOMPD
    /* generate dummy data */
    struct mpdisplay_mpd_status *st = mpdisplay_mpd_status_new ();

    st->success = true;
    mpdisplay_mpd_status_add_song_data (st, "tag1", "value1");
    mpdisplay_mpd_status_add_song_data (st, "tag2", "value2 with more content");
    mpdisplay_mpd_status_add_song_data (st, "tag3", "value3 with more so much content that it should be necessary to wrap it somewhere");
#else
    /* get status */
    struct mpdisplay_mpd_status *st = mpdisplay_mpd_get_status ();
#endif

    update_mpd_status (st);
}
