## Process this file with automake to produce Makefile.in

## Created by Anjuta

SUBDIRS = garglk frotz glulxe 

componentdocdir = ${datadir}/doc/gtkterp
componentdoc_DATA = \
	AUTHORS-frotz README-frotz COPYING-frotz TODO-frotz \
	README-glulxe TODO-garglk \
	CharterBT.txt LuxiMono.txt \
	License-gargoyle.txt

EXTRA_DIST = $(componentdoc_DATA)

INCLUDES = \
	-DPACKAGE_LOCALE_DIR=\""$(prefix)/$(DATADIRNAME)/locale"\" \
	-DPACKAGE_SRC_DIR=\""$(srcdir)"\" \
	-DPACKAGE_DATA_DIR=\""$(datadir)"\" \
	$(GTKTERP_CFLAGS)

AM_CFLAGS =\
	 -Wall\
	 -g

noinst_LIBRARIES = libgtkterp.a
libgtkterp_a_SOURCES = gtkterp.c gtkterp.h
libgtkterp_a_CFLAGS = $(GTKTERP_CFLAGS)

BUILT_SOURCES = dbus-client.h
noinst_HEADERS = $(BUILT_SOURCES)
CLEANFILES = $(BUILT_SOURCES)

dbus-client.h: garglk/gtkterp-dbus.xml
	dbus-binding-tool --prefix=garglk --mode=glib-client $< --output=$@

