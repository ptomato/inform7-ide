SUBDIRS = src po data pixmaps

desktopdir = $(datadir)/applications
dist_desktop_DATA = gnome-inform7.desktop

schemadir = @GCONF_SCHEMA_FILE_DIR@
dist_schema_DATA = gnome-inform7.schemas

docdir = $(datadir)/doc/$(PACKAGE)
doc_DATA = AUTHORS ChangeLog COPYING NEWS README THANKS TODO

install-data-local:
	GCONF_CONFIG_SOURCE=$(GCONF_SCHEMA_CONFIG_SOURCE) \
	$(GCONFTOOL) --makefile-install-rule $(srcdir)/$(dist_schema_DATA)

uninstall-local:
	GCONF_CONFIG_SOURCE=$(GCONF_SCHEMA_CONFIG_SOURCE) \
	$(GCONFTOOL) --makefile-uninstall-rule $(srcdir)/$(dist_schema_DATA)

EXTRA_DIST = \
	autogen.sh \
	intltool-update.in \
	intltool-extract.in \
	intltool-merge.in \
	gnome-inform7.glade \
	gnome-inform7.gladep \
	gnome-inform7.spec.in
	
if BUILDING_RPM
EXTRA_DIST += gnome-inform7.spec
endif

DISTCLEANFILES = \
	intltool-extract \
	intltool-merge \
	intltool-update \
	gnome-inform7.spec

if BUILDING_RPM
RPMDIR=`$(RPM) --eval %_rpmdir`
SRCRPMDIR=`$(RPM) --eval %_srcrpmdir`
RPMARCH=`$(RPM) --eval %_arch`
BINRPM = $(PACKAGE)-$(VERSION)-$(RPM_RELEASE).$(RPMARCH).rpm
DEBUGRPM = $(PACKAGE)-debuginfo-$(VERSION)-$(RPM_RELEASE).$(RPMARCH).rpm
SRCRPM = $(PACKAGE)-$(VERSION)-$(RPM_RELEASE).src.rpm
rpm: dist
	$(ECHO) "Building RPM package..."; \
	$(RPMBUILD) -ta --clean $(PACKAGE)-$(VERSION).tar.gz && \
	$(CP) $(RPMDIR)/$(BINRPM) $(srcdir) && \
	$(CP) $(SRCRPMDIR)/$(SRCRPM) $(srcdir) && \
	$(TEST) -e $(RPMDIR)/$(DEBUGRPM) && \
	$(CP) $(RPMDIR)/$(DEBUGRPM) $(srcdir)
else
rpm:
	@$(ECHO) \
	"To build an RPM package, you must run configure with --enable-rpm."; \
	exit 1
endif
