SUBDIRS = garglk frotz glulxe git 

# This line won't be necessary anymore in Automake 1.10a
pkglibexecdir = $(libexecdir)/$(PACKAGE)

AM_CFLAGS = -Wall

noinst_LIBRARIES = libgtkterp.a
libgtkterp_a_SOURCES = gtkterp.c gtkterp.h
libgtkterp_a_CPPFLAGS = \
	-DPACKAGE_LOCALE_DIR=\""$(prefix)/$(DATADIRNAME)/locale"\" \
	-DPACKAGE_LIBEXEC_DIR=\""$(pkglibexecdir)"\" \
	-DPACKAGE_DATA_DIR=\""$(datadir)"\"
libgtkterp_a_CFLAGS = @GTKTERP_CFLAGS@ $(AM_CFLAGS)

noinst_HEADERS =  dbus-client.h
BUILT_SOURCES = $(noinst_HEADERS)
$(noinst_HEADERS): garglk/gtkterp-dbus.xml
	dbus-binding-tool --prefix=garglk --mode=glib-client $< --output=$@

CLEANFILES = $(noinst_HEADERS)

