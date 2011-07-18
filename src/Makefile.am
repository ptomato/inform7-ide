SUBDIRS = inweb cBlorb inform6 ni libchimara #interpreters

AM_CFLAGS = -Wall

# This line won't be necessary anymore in Automake 1.10a
pkglibexecdir = $(libexecdir)/$(PACKAGE)

noinst_LIBRARIES = libosxcart.a
libosxcart_a_SOURCES = \
	osxcart/osxcart/init.c \
	osxcart/osxcart/init.h \
	osxcart/osxcart/plist.c \
	osxcart/osxcart/plist.h \
	osxcart/osxcart/plist-common.c \
	osxcart/osxcart/plist-common.h \
	osxcart/osxcart/plist-write.c \
	osxcart/osxcart/plist-parser.c \
	osxcart/osxcart/rtf.c \
	osxcart/osxcart/rtf.h \
	osxcart/osxcart/rtf-serialize.c \
	osxcart/osxcart/rtf-serialize.h \
	osxcart/osxcart/rtf-deserialize.c \
	osxcart/osxcart/rtf-deserialize.h \
	osxcart/osxcart/rtf-document.c \
	osxcart/osxcart/rtf-document.h \
	osxcart/osxcart/rtf-fonttbl.c \
	osxcart/osxcart/rtf-colortbl.c \
	osxcart/osxcart/rtf-ignore.c \
	osxcart/osxcart/rtf-ignore.h \
	osxcart/osxcart/rtf-footnote.c \
	osxcart/osxcart/rtf-picture.c \
	osxcart/osxcart/rtf-field.c \
	osxcart/osxcart/rtf-stylesheet.c \
	osxcart/osxcart/rtf-langcode.c \
	osxcart/osxcart/rtf-langcode.h \
	osxcart/osxcart/rtf-state.c \
	osxcart/osxcart/rtf-state.h
libosxcart_a_CPPFLAGS = \
	-DG_LOG_DOMAIN=\"Osxcart\" \
	-DLOCALEDIR=\""$(datadir)/locale"\" \
	-I$(srcdir)/osxcart
libosxcart_a_CFLAGS = @OSXCART_CFLAGS@ $(AM_CFLAGS)

bin_PROGRAMS = gnome-inform7
gnome_inform7_SOURCES = \
	actions.c actions.h \
	app.c app.h app-private.h \
	builder.c builder.h \
	colorscheme.c colorscheme.h \
	configfile.c configfile.h \
	document.c document.h document-private.h \
	document-search.c \
	elastic.c elastic.h \
	error.c error.h \
	extension.c extension.h \
	file.c file.h \
	history.c history.h \
	html.c html.h \
	lang.c lang.h \
	newdialog.c newdialog.h \
	node.c node.h \
	notepad.c \
	panel.c panel.h panel-private.h \
	placeholder-entry.c placeholder-entry.h \
	prefs.c prefs.h \
	searchwindow.c searchwindow.h \
	skein.c skein.h \
	skein-view.c skein-view.h \
	source-view.c source-view.h \
	spawn.c spawn.h \
	story.c story.h story-private.h \
	story-source.c story-errors.c story-index.c story-settings.c \
	story-compile.c story-game.c story-skein.c \
	welcomedialog.c welcomedialog.h \
	main.c
gnome_inform7_CPPFLAGS = \
	-DPACKAGE_DATA_DIR=\""$(datadir)"\" \
	-DPACKAGE_LIBEXEC_DIR=\""$(pkglibexecdir)"\" \
	-DPACKAGE_LOCALE_DIR=\""$(prefix)/$(DATADIRNAME)/locale"\" \
	-I$(srcdir)/osxcart
gnome_inform7_CFLAGS = -g @INFORM7_CFLAGS@ $(AM_CFLAGS)
gnome_inform7_LDADD = \
	@INFORM7_LIBS@ @OSXCART_LIBS@ @CHIMARA_LIBS@ \
	$(INTLLIBS) \
	libosxcart.a \
	libchimara/libchimara.a
# The following mystical incantation is needed because the Glk programs, which
# are dynamically loaded, reference symbols from the Glk API in libchimara.a
# which are not referenced by the main program. The default behavior of ld is
# not to include object files with no referenced symbols in the final binary
# product. This tells ld to link in all object files in libchimara.a.
gnome_inform7_LDFLAGS = -Wl,--export-dynamic \
	-Wl,--whole-archive,libchimara/libchimara.a,--no-whole-archive

#noinst_PROGRAMS = skeintest
#skeintest_SOURCES = skeintest.c skein.c skein.h node.c node.h skein-view.c skein-view.h
#skeintest_CFLAGS = @INFORM7_CFLAGS@ $(AM_CFLAGS)
#skeintest_LDADD = @INFORM7_LIBS@

-include $(top_srcdir)/git.mk
