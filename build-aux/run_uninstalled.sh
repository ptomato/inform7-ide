#!/bin/sh

ninja -C _build || exit 1
DESTDIR=$(pwd)/_install ninja -C _build install >/dev/null || exit 1
INFORM7_IDE_DATA_DIR=_install/usr/local/share/inform7-ide \
    GSETTINGS_SCHEMA_DIR=_build \
    INFORM7_IDE_LIBEXEC_DIR=_install/usr/local/libexec/inform7-ide \
    LD_LIBRARY_PATH=_build/subprojects/chimara/libchimara:_build/subprojects/ratify \
    $DEBUGGER _build/src/inform7-ide
