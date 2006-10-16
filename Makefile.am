## Process this file with automake to produce Makefile.in

SUBDIRS = src po data

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
	fi; \
	pushd $(DESTDIR)$(pkgdatadir)/extensions/Emily\ Short; \
	mv BasicHelpMenu Basic\ Help\ Menu; \
	mv BasicScreenEffects Basic\ Screen\ Effects; \
	mv CaseManagement Case\ Management; \
	mv ComplexListing Complex\ Listing; \
	mv LiddedChests Lidded\ Chests; \
	mv PunctuationRemoval Punctuation\ Removal; \
	mv StopperedBottles Stoppered\ Bottles; \
	cd ../Graham\ Nelson; \
	mv RideableVehicles Rideable\ Vehicles; \
	mv StandardRules Standard\ Rules; \
	mv UnicodeCharacterNames Unicode\ Character\ Names; \
	mv UnicodeFullCharacterNames Unicode\ Full\ Character\ Names; \
	popd
	
uninstall-local:
	pushd $(DESTDIR)$(pkgdatadir)/extensions/Emily\ Short; \
	rm -f Basic\ Help\ Menu; \
	rm -f Basic\ Screen\ Effects; \
	rm -f Case\ Management; \
	rm -f Complex\ Listing; \
	rm -f Lidded\ Chests; \
	rm -f Punctuation\ Removal; \
	rm -f Stoppered\ Bottles; \
	cd ../Graham\ Nelson; \
	rm -f Rideable\ Vehicles; \
	rm -f Standard\ Rules; \
	rm -f Unicode\ Character\ Names; \
	rm -f Unicode\ Full\ Character\ Names; \
	popd

dist-hook:
	if test -d pixmaps; then \
	  mkdir $(distdir)/pixmaps; \
	  for pixmap in pixmaps/*; do \
	    if test -f $$pixmap; then \
	      cp -p $$pixmap $(distdir)/pixmaps; \
	    fi \
	  done \
	fi
	
