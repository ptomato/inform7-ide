#!/bin/sh

ninja -C _build
DESTDIR=$(pwd)/_install ninja -C _build install >/dev/null
GNOME_INFORM_DATA_DIR=_install/usr/local/share/gnome-inform7 \
    GNOME_INFORM_LIBEXEC_DIR=_install/usr/local/libexec/gnome-inform7 \
    LD_LIBRARY_PATH=_build/subprojects/chimara/libchimara:_build/subprojects/ratify \
    _build/src/gnome-inform7
