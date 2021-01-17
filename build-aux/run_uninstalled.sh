#!/bin/sh

ninja -C _build || exit 1
DESTDIR=$(pwd)/_install ninja -C _build install >/dev/null || exit 1
GNOME_INFORM_DATA_DIR=_install/usr/local/share/gnome-inform7 \
    GSETTINGS_SCHEMA_DIR=_build \
    GNOME_INFORM_LIBEXEC_DIR=_install/usr/local/libexec/gnome-inform7 \
    LD_LIBRARY_PATH=_build/subprojects/chimara/libchimara:_build/subprojects/ratify \
    $DEBUGGER _build/src/gnome-inform7
