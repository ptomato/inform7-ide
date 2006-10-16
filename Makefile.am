## Process this file with automake to produce Makefile.in

## SUBDIRS = src po data
SUBDIRS = src po

EXTRA_DIST = \
	autogen.sh \
	gnome-inform7.glade \
	gnome-inform7.gladep \
	gnome-inform7.desktop \
	gnome-inform7.spec

desktopdir = $(datadir)/applications
desktop_DATA = gnome-inform7.desktop

appicondir = $(datadir)/pixmaps
appicon_DATA = pixmaps/Inform.png

install-data-local:
	@$(NORMAL_INSTALL)
	if test -d $(srcdir)/pixmaps; then \
	  $(mkinstalldirs) $(DESTDIR)$(datadir)/pixmaps/$(PACKAGE); \
	  for pixmap in $(srcdir)/pixmaps/*; do \
	    if test -f $$pixmap; then \
	      $(INSTALL_DATA) $$pixmap $(DESTDIR)$(datadir)/pixmaps/$(PACKAGE); \
	    fi \
	  done \
	fi; 

dist-hook:
	if test -d pixmaps; then \
	  mkdir $(distdir)/pixmaps; \
	  for pixmap in pixmaps/*; do \
	    if test -f $$pixmap; then \
	      cp -p $$pixmap $(distdir)/pixmaps; \
	    fi \
	  done \
	fi
	
