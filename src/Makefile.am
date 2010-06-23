SUBDIRS = inweb cBlorb inform6 gtkterp ni

AM_CFLAGS = -Wall

# This line won't be necessary anymore in Automake 1.10a
pkglibexecdir = $(libexecdir)/$(PACKAGE)

bin_PROGRAMS = gnome-inform7
gnome_inform7_SOURCES = \
	gtkterp/gtkterp.h \
	main.c \
	support.c support.h \
	interface.c interface.h \
	callbacks.c callbacks.h \
	newdialog.c newdialog.h \
	story.c story.h \
	tabindex.c tabindex.h \
	tabsettings.c tabsettings.h \
	tabsource.c tabsource.h \
	file.c file.h \
	lang.c lang.h \
	findreplace.c findreplace.h \
	appwindow.c appwindow.h \
	welcomedialog.c welcomedialog.h \
	html.c html.h \
	configfile.c configfile.h \
    extension.c extension.h \
    extwindow.c extwindow.h \
    windowlist.c windowlist.h \
    prefs.c prefs.h \
    colorscheme.c colorscheme.h \
    compile.c compile.h \
    tabgame.c tabgame.h \
    error.c error.h \
	inspector.c inspector.h \
    rtf.c rtf.h \
    searchwindow.c searchwindow.h \
    taberrors.c taberrors.h \
    datafile.c datafile.h \
    spawn.c spawn.h \
    widgets.c widgets.h \
    history.c history.h \
    skein.c skein.h \
    tabskein.c tabskein.h \
    appmenu.c appmenu.h \
    apptoolbar.c apptoolbar.h \
    tabdocs.c tabdocs.h \
	elastic.c elastic.h
gnome_inform7_CPPFLAGS = \
	-DPACKAGE_DATA_DIR=\""$(datadir)"\" \
	-DPACKAGE_LIBEXEC_DIR=\""$(pkglibexecdir)"\" \
	-DPACKAGE_LOCALE_DIR=\""$(prefix)/$(DATADIRNAME)/locale"\"
gnome_inform7_CFLAGS = @GNOMEINFORM7_CFLAGS@ $(AM_CFLAGS)
gnome_inform7_LDADD = \
	@GNOMEINFORM7_LIBS@ \
	$(INTLLIBS) \
	@GTKTERP_LIBS@ gtkterp/libgtkterp.a
gnome_inform7_LDFLAGS = -L.

