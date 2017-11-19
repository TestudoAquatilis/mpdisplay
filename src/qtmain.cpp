#include <QApplication>

#include "win_disp.h"
#include "options.h"
#include "mpd.h"

int main(int argc, char *argv[])
{
    if (!mpdisplay_parse_options (&argc, &argv)) return 1;

    QApplication app(argc, argv);

    WinDisp win_disp;
    win_disp.show();

    int result = app.exec();

    mpdisplay_mpd_free ();

    return result;
}
