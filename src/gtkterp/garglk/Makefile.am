## Process this file with automake to produce Makefile.in

## Created by Anjuta

#SUBDIRS = fonts

INCLUDES = \
	-DPACKAGE_LOCALE_DIR=\""$(prefix)/$(DATADIRNAME)/locale"\" \
	-DPACKAGE_SRC_DIR=\""$(srcdir)"\" \
	-DPACKAGE_DATA_DIR=\""$(datadir)"\" \
	$(GTKTERP_CFLAGS)

AM_CFLAGS = -Wall

noinst_LIBRARIES = libgarglk.a

libgarglk_a_SOURCES = gi_blorb.c gi_dispa.c nohyper.c cggestal.c cgblorb.c \
	cgfref.c cgmisc.c cgstyle.c cgstream.c window.c winblank.c winpair.c \
	wingrid.c wintext.c wingfx.c event.c draw.c config.c imgload.c \
	imgscale.c fontdata.c sndsdl.c sysgtk.c main.c garglk.h garversion.h \
	gi_blorb.h gi_dispa.h glk.h glkstart.h garglk-plug.h garglk-plug.c sysgtk.h

BUILT_SOURCES = dbus-server.h
noinst_HEADERS = $(BUILT_SOURCES)
CLEANFILES = $(BUILT_SOURCES)

dbus-server.h: gtkterp-dbus.xml
	dbus-binding-tool --prefix=garglk --mode=glib-server $< --output=$@

EXTRA_DIST = cbb.hex cbi.hex cbr.hex cbz.hex lmb.hex lmi.hex lmr.hex lmz.hex \
	garglk.ini gtkterp-dbus.xml
