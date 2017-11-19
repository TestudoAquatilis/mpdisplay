#include <QApplication>

#include "win_disp.h"
#include "options.h"
#include "mpd.h"

int main(int argc, char *argv[])
{
    if (!mpdisplay_parse_options (&argc, &argv)) return 1;

    QApplication app(argc, argv);

    WinDisp win_disp;

    if (mpdisplay_options.win_fullscreen) {
        win_disp.showMaximized();
    } else {
        win_disp.show();
    }

    if (!mpdisplay_options.win_cursor) {
        app.setOverrideCursor (Qt::BlankCursor);
    }

    int result = app.exec();

    mpdisplay_mpd_free ();

    return result;
}
