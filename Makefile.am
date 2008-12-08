## Process this file with automake to produce Makefile.in

SUBDIRS = src po data debian

EXTRA_DIST = \
	autogen.sh \
	intltool-update.in \
	intltool-extract.in \
	intltool-merge.in \
	gnome-inform7.glade \
	gnome-inform7.gladep \
	gnome-inform7.desktop \
	gnome-inform7.spec \
	gnome-inform7.schemas

desktopdir = $(datadir)/applications
desktop_DATA = gnome-inform7.desktop

appicondir = $(datadir)/pixmaps
appicon_DATA = pixmaps/Inform.png

schemadir = @GCONF_SCHEMA_FILE_DIR@
schema_DATA = gnome-inform7.schemas

install-data-local:
	@$(NORMAL_INSTALL)
	if test -d $(srcdir)/pixmaps; then \
	  $(mkinstalldirs) $(DESTDIR)$(datadir)/pixmaps/$(PACKAGE); \
	  for pixmap in $(srcdir)/pixmaps/*; do \
	    if test -f $$pixmap; then \
	      $(INSTALL_DATA) $$pixmap $(DESTDIR)$(datadir)/pixmaps/$(PACKAGE); \
	    fi \
	  done \
	fi; \
	GCONF_CONFIG_SOURCE=$(GCONF_SCHEMA_CONFIG_SOURCE) \
	$(GCONFTOOL) --makefile-install-rule $(srcdir)/$(schema_DATA)

dist-hook:
	if test -d pixmaps; then \
	  mkdir $(distdir)/pixmaps; \
	  for pixmap in pixmaps/*; do \
	    if test -f $$pixmap; then \
	      cp -p $$pixmap $(distdir)/pixmaps; \
	    fi \
	  done \
	fi

uninstall-local:
	GCONF_CONFIG_SOURCE=$(GCONF_SCHEMA_CONFIG_SOURCE) \
	$(GCONFTOOL) --makefile-uninstall-rule $(srcdir)/$(schema_DATA)
