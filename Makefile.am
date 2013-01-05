SUBDIRS = src po data pixmaps

ACLOCAL_AMFLAGS = -I m4

desktopdir = $(datadir)/applications
dist_desktop_DATA = gnome-inform7.desktop

schemadir = @GCONF_SCHEMA_FILE_DIR@
dist_schema_DATA = gnome-inform7.schemas

docdir = $(datadir)/doc/$(PACKAGE)
dist_doc_DATA = AUTHORS ChangeLog COPYING NEWS README THANKS TODO

install-data-local:
	GCONF_CONFIG_SOURCE=$(GCONF_SCHEMA_CONFIG_SOURCE) \
	$(GCONFTOOL) --makefile-install-rule $(srcdir)/$(dist_schema_DATA)
	killall -HUP gconfd-2  # Refresh gconfd for all users, not just root

uninstall-local:
	GCONF_CONFIG_SOURCE=$(GCONF_SCHEMA_CONFIG_SOURCE) \
	$(GCONFTOOL) --makefile-uninstall-rule $(srcdir)/$(dist_schema_DATA)
	killall -HUP gconfd-2

EXTRA_DIST = \
	INSTALL \
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

MAINTAINERCLEANFILES = ABOUT-NLS INSTALL aclocal.m4 compile config.guess \
        config.h.in config.rpath config.sub depcomp install-sh \
        intltool-extract.in intltool-merge.in intltool-update.in ltmain.sh \
        missing autom4te.cache m4

if BUILDING_RPM
RPMDIR=`$(RPM) --eval %_rpmdir`
SRCRPMDIR=`$(RPM) --eval %_srcrpmdir`
RPMARCH=`$(RPM) --eval %_target_cpu`
BINRPM = $(PACKAGE)-$(VERSION)-$(RPM_RELEASE).$(RPMARCH).rpm
DEBUGRPM = $(PACKAGE)-debuginfo-$(VERSION)-$(RPM_RELEASE).$(RPMARCH).rpm
SRCRPM = $(PACKAGE)-$(VERSION)-$(RPM_RELEASE).src.rpm
rpm: dist
	$(ECHO) "Building RPM package..."; \
	if $(RPMBUILD) -ta --clean $(PACKAGE)-$(VERSION).tar.gz; then \
		if $(TEST) -e $(RPMDIR)/$(BINRPM); then \
			$(CP) $(RPMDIR)/$(BINRPM) $(builddir); \
		elif $(TEST) -e $(RPMDIR)/$(RPMARCH)/$(BINRPM); then \
			$(CP) $(RPMDIR)/$(RPMARCH)/$(BINRPM) $(builddir); \
		fi; \
		$(TEST) -e $(SRCRPMDIR)/$(SRCRPM) && \
		$(CP) $(SRCRPMDIR)/$(SRCRPM) $(builddir); \
		if $(TEST) -e $(RPMDIR)/$(DEBUGRPM); then \
			$(CP) $(RPMDIR)/$(DEBUGRPM) $(builddir); \
		elif $(TEST) -e $(RPMDIR)/$(RPMARCH)/$(DEBUGRPM); then \
			$(CP) $(RPMDIR)/$(RPMARCH)/$(DEBUGRPM) $(builddir); \
		else \
			$(ECHO) "No debug package; install redhat-rpm-config if you want one."; \
		fi; \
	fi
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
	@$(ECHO) "Building Debian package..."; \
	if $(TEST) ! -d debian; then \
		$(ECHO) "The files needed for building a Debian package are not" \
		"included by default in the distribution. To build a package, check" \
		"out the project from source control."; \
		exit 1; \
	fi; \
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

-include $(top_srcdir)/git.mk
