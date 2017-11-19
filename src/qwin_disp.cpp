#include <QtWidgets>
#include <glib.h>

#include "qwin_disp.h"
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
    /* main layout elements */
    QWidget *top_row   = create_top_row ();
    QWidget *top_line  = create_new_separator_line ();
    QWidget *cen_frame = create_center_frame ();
    QWidget *bot_line  = create_new_separator_line ();
    QWidget *bot_row   = create_bottom_row ();

    /* arrange */
    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(top_row,   0);
    mainLayout->addWidget(top_line,  0);
    mainLayout->addWidget(cen_frame, 1);
    mainLayout->addWidget(bot_line,  0);
    mainLayout->addWidget(bot_row,   0);

    /* insert */
    QWidget *mainWidget = new QWidget;
    mainWidget->setLayout(mainLayout);
    setCentralWidget (mainWidget);

    /* title */
    setWindowTitle("MPDisplay");

    /* size */
    int w_w = mpdisplay_options.win_width;
    int w_h = mpdisplay_options.win_height;
    if ((w_w > 0) | (w_h > 0)) {
        this->resize (w_w, w_h);
    }
}

QWidget *WinDisp::create_top_row ()
{
    QWidget *result = new QWidget;

    /* icon */
    lb_state = new QLabel;
    update_playback_state (ST_STOP);

    /* time bar */
    pb_time = new QProgressBar;
    pb_time->setFormat ("?");
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
    int icon_size = QApplication::style()->pixelMetric(QStyle::PM_SmallIconSize);
    if (mpdisplay_options.icon_size_toolbar > 0) icon_size = mpdisplay_options.icon_size_toolbar;
    QSize qicon_size = QSize (icon_size, icon_size);

    /* tool buttons */
    tb_single  = new QToolButton;
    tb_repeat  = new QToolButton;
    tb_shuffle = new QToolButton;

    tb_single->setText ("1");
    tb_repeat->setIcon (QIcon::fromTheme ("media-playlist-repeat-symbolic"));
    tb_shuffle->setIcon (QIcon::fromTheme ("media-playlist-shuffle-symbolic"));
    tb_repeat->setIconSize (qicon_size);
    tb_shuffle->setIconSize (qicon_size);

    tb_single->setCheckable(true);
    tb_repeat->setCheckable(true);
    tb_shuffle->setCheckable(true);

    /* volume icon */
    QLabel *lb_volume = new QLabel;
    lb_volume->setPixmap (QIcon::fromTheme ("player-volume").pixmap (qicon_size));

    /* volume bar */
    pb_volume = new QProgressBar;
    pb_volume->setFormat ("%p\%");
    pb_volume->setTextVisible (true);

    pb_volume->setMinimum (0);
    pb_volume->setMaximum (100);
    pb_volume->setValue (0);

    /* put everything together */
    QHBoxLayout *layout = new QHBoxLayout;
    layout->addWidget (tb_single, 0);
    layout->addWidget (tb_repeat, 0);
    layout->addWidget (tb_shuffle, 0);
    layout->addStretch (1);
    layout->addWidget (lb_volume, 0);
    layout->addWidget (pb_volume, 0);

    result->setLayout (layout);

    return result;
}

QWidget *WinDisp::create_center_frame ()
{
    /* create frame */
    fr_center = new QFrame;
    ly_center = new QFormLayout;

    fr_center->setFrameShape (QFrame::NoFrame);
    fr_center->setLayout     (ly_center);

    /* fill with no-connection message */
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
    const char *st_string_stop  = "media-playback-stop-symbolic";
    const char *st_string_play  = "media-playback-start-symbolic";
    const char *st_string_pause = "media-playback-pause-symbolic";

    const char *st_string = st_string_stop;

    if (s == ST_PLAY) {
        st_string = st_string_play;
    } else if (s == ST_PAUSE) {
        st_string = st_string_pause;
    }

    int icon_size = QApplication::style()->pixelMetric(QStyle::PM_MessageBoxIconSize);
    if (mpdisplay_options.icon_size_playback > 0) icon_size = mpdisplay_options.icon_size_playback;
    lb_state->setPixmap (QIcon::fromTheme (st_string).pixmap (icon_size, icon_size));
}

void WinDisp::update_playback_state (struct mpdisplay_mpd_status *st)
{
    if ((st == NULL) || (!st->success)) {
        update_playback_state (ST_STOP);
        return;
    }

    struct mpdisplay_mpd_status *cst = mpd_st_current;

    if ((cst != NULL) && (cst->success)) {
        if ((cst->play == st->play) && (cst->pause == st->pause)) return;
    }

    if (st->play) {
        update_playback_state (ST_PLAY);
    } else if (st->pause) {
        update_playback_state (ST_PAUSE);
    } else {
        update_playback_state (ST_STOP);
    }
}

void WinDisp::update_playlist_state (bool single, bool repeat, bool shuffle)
{
    tb_single->setChecked (single);
    tb_repeat->setChecked (repeat);
    tb_shuffle->setChecked (shuffle);
}

void WinDisp::update_playlist_state (struct mpdisplay_mpd_status *st)
{
    if ((st == NULL) || (!st->success)) {
        update_playlist_state (false, false, false);
        return;
    }

    struct mpdisplay_mpd_status *cst = mpd_st_current;

    if ((cst != NULL) && (cst->success)) {
        if ((cst->single == st->single) && (cst->repeat == st->repeat) && (cst->shuffle == st->shuffle)) return;
    }

    update_playlist_state (st->single, st->repeat, st->shuffle);
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

void WinDisp::update_volume (struct mpdisplay_mpd_status *st)
{
    if ((st == NULL) || (!st->success)) {
        update_volume (0);
        return;
    }

    struct mpdisplay_mpd_status *cst = mpd_st_current;

    if ((cst != NULL) && (cst->success)) {
        if (cst->volume == st->volume) return;
    }

    update_volume (st->volume);
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

void WinDisp::update_time (struct mpdisplay_mpd_status *st)
{
    if ((st == NULL) || (!st->success)) {
        update_time (0, 0);
        return;
    }

    struct mpdisplay_mpd_status *cst = mpd_st_current;

    if ((cst != NULL) && (cst->success)) {
        if ((cst->seconds_elapsed == st->seconds_elapsed) && (cst->seconds_total == st->seconds_total)) return;
    }

    update_time (st->seconds_total, st->seconds_elapsed);
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

void WinDisp::update_tags (GList *tlist)
{
    clear_tags ();

    for (GList *li = tlist; li != NULL; li = li->next) {
        struct mpdisplay_song_data_entry *e = static_cast<struct mpdisplay_song_data_entry *>(li->data);

        QLabel *lbl_name  = new QLabel (e->name);
        QLabel *lbl_value = new QLabel (e->value);

        lbl_name->setAlignment  (Qt::AlignLeft    | Qt::AlignTop);
        lbl_value->setAlignment (Qt::AlignHCenter | Qt::AlignVCenter);

        lbl_value->setWordWrap (true);

        if (mpdisplay_options.win_prioscale) {
            if (e->priority < 0) {
                QFont font = lbl_name->font();

                int psp = font.pointSize();
                if (psp <= 0) {
                    qreal psf = font.pointSizeF();
                    if (psf > 0) {
                        font.setPointSizeF ((psf * 2)/3.0);
                    }
                } else {
                    font.setPointSize ((psp * 2)/3);
                }

                lbl_name->setFont (font);
                lbl_value->setFont (font);
            }
        }

        ly_center->addRow (lbl_name, lbl_value);
    }
}

void WinDisp::update_tags (struct mpdisplay_mpd_status *st)
{
    if ((st == NULL) || (!st->success)) {
        update_tags_nocon ();
        return;
    }

    struct mpdisplay_mpd_status *cst = mpd_st_current;

    if ((cst != NULL) && (cst->success)) {
        if (mpdisplay_mpd_status_tags_equal (cst, st)) return;
    }

    update_tags (st->song_data->head);
}

void WinDisp::update_mpd_status (struct mpdisplay_mpd_status *st_new)
{
    update_playback_state (st_new);
    update_playlist_state (st_new);
    update_volume (st_new);
    update_time (st_new);
    update_tags (st_new);

    mpdisplay_mpd_status_free (&mpd_st_current);
    mpd_st_current = mpdisplay_mpd_status_copy (st_new);
}

void WinDisp::update_mpd_status ()
{
#ifdef DEBUG_NOMPD
    /* generate dummy data */
    struct mpdisplay_mpd_status *st = mpdisplay_mpd_status_new ();

    st->success = true;
    mpdisplay_mpd_status_add_song_data (st, "tag1", "value1", 0);
    mpdisplay_mpd_status_add_song_data (st, "tag2", "value2 with more content", -1);
    mpdisplay_mpd_status_add_song_data (st, "tag3", "value3 with more so much content that it should be necessary to wrap it somewhere", 0);
#else
    /* get status */
    struct mpdisplay_mpd_status *st = mpdisplay_mpd_get_status ();
#endif

    update_mpd_status (st);

    mpdisplay_mpd_status_free (&st);
}
