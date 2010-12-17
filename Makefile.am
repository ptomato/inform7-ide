SUBDIRS = src po data pixmaps

ACLOCAL_AMFLAGS = -I m4

desktopdir = $(datadir)/applications
dist_desktop_DATA = gnome-inform7.desktop

schemadir = @GCONF_SCHEMA_FILE_DIR@
dist_schema_DATA = gnome-inform7.schemas

docdir = $(datadir)/doc/$(PACKAGE)
dist_doc_DATA = AUTHORS ChangeLog COPYING INSTALL NEWS README THANKS TODO

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
RPMARCH=`$(RPM) --eval %_target_cpu`
BINRPM = $(PACKAGE)-$(VERSION)-$(RPM_RELEASE).$(RPMARCH).rpm
DEBUGRPM = $(PACKAGE)-debuginfo-$(VERSION)-$(RPM_RELEASE).$(RPMARCH).rpm
SRCRPM = $(PACKAGE)-$(VERSION)-$(RPM_RELEASE).src.rpm
rpm: dist
	$(ECHO) "Building RPM package..."; \
	$(RPMBUILD) -ta --clean $(PACKAGE)-$(VERSION).tar.gz && \
	$(CP) $(RPMDIR)/$(RPMARCH)/$(BINRPM) $(srcdir); \
	$(CP) $(SRCRPMDIR)/$(SRCRPM) $(srcdir) && \
	$(TEST) -e $(RPMDIR)/$(RPMARCH)/$(DEBUGRPM) && \
	$(CP) $(RPMDIR)/$(RPMARCH)/$(DEBUGRPM) $(srcdir) || \
	$(ECHO) "No debug package; install redhat-rpm-config if you want one."
else
rpm:
	@$(ECHO) \
	"To build an RPM package, you must run configure with --enable-rpm."; \
	exit 1
endif

if BUILDING_DEB
DEBSOURCEPKG=$(PACKAGE)_$(VERSION).orig.tar.gz
DEBSOURCEDIR=$(PACKAGE)-$(VERSION)
deb: dist
	$(ECHO) "Building Debian package..."; \
	$(MV) $(PACKAGE)-$(VERSION).tar.gz $(DEBSOURCEPKG) && \
	$(TAR) --extract --gunzip --file $(DEBSOURCEPKG) && \
	$(MKDIR_P) $(DEBSOURCEDIR)/debian && \
	$(CP) \
		debian/changelog \
		debian/compat \
		debian/control \
		debian/copyright \
		debian/docs \
		debian/menu \
		debian/rules \
		$(DEBSOURCEDIR)/debian && \
	cd $(DEBSOURCEDIR) && \
	$(DEBUILD) -rfakeroot -D -us -uc; \
	cd $(srcdir); \
	rm --force $(DEBSOURCEPKG); \
	rm --recursive --force $(DEBSOURCEDIR)
else
deb:
	@$(ECHO) \
	"To build a Debian package, you must run configure with --enable-deb."; \
	exit 1
endif
