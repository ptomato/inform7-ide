dist_sysconf_DATA = gtkterp.ini

noinst_LIBRARIES = libgarglk.a
libgarglk_a_SOURCES = gi_blorb.c gi_dispa.c cggestal.c cgblorb.c cgfref.c \
	cgmisc.c cgstyle.c cgstream.c cgunicod.c window.c winblank.c winpair.c \
	wingrid.c wintext.c wingfx.c event.c draw.c config.c imgload.c imgscale.c \
	fontdata.c sndsdl.c sysgtk.c main.c garglk.h garversion.h gi_blorb.h \
	gi_dispa.h glk.h glkstart.h garglk-plug.h garglk-plug.c sysgtk.h winmask.c
libgarglk_a_CPPFLAGS = \
	-DPACKAGE_LOCALE_DIR=\""$(prefix)/$(DATADIRNAME)/locale"\" \
	-DPACKAGE_SRC_DIR=\""$(srcdir)"\" \
	-DPACKAGE_DATA_DIR=\""$(datadir)"\"
libgarglk_a_CFLAGS = @GTKTERP_CFLAGS@

noinst_HEADERS = dbus-server.h
BUILT_SOURCES = $(noinst_HEADERS)
$(noinst_HEADERS): gtkterp-dbus.xml
	dbus-binding-tool --prefix=garglk --mode=glib-server $< --output=$@

garglkdocdir = $(datadir)/doc/$(PACKAGE)/garglk
dist_garglkdoc_DATA = CharterBT.txt License.txt LuxiMono.txt TODO

CLEANFILES = $(noinst_HEADERS)

EXTRA_DIST = cbb.hex cbi.hex cbr.hex cbz.hex lmb.hex lmi.hex lmr.hex lmz.hex \
	gtkterp-dbus.xml cgunigen.c

