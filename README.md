# mpdisplay

mpdisplay is an MPD client for displaying current status on small displays (eg ARM-boards + TFT)

# Requirements

You need
- libmpdclient
- glib version 2

and depending on GUI variant either of
- gtk+ 3.0
- Qt 5.9

# Build

In src directory run:

> VARIANT=gtk make

or

> VARIANT=Qt make

# Usage

Just start the executable (gmpdisplay for gtk-variant, qmpdisplay for Qt-variant).
Options can be specified via a config file (example in cfg) or as command line arguments.

