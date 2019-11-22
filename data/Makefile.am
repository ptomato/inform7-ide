SUBDIRS = . highlighting mime styles

INFORMTARBALL = Inform_@PACKAGE_VERSION@_data.tar.lz

EXTRA_DIST = $(INFORMTARBALL)

EXCLUDE_ARGS = --exclude=Documentation* --exclude=Resources*

install-data-hook:
	$(MKDIR_P) $(DESTDIR)$(pkgdatadir) && \
	cd $(DESTDIR)$(pkgdatadir) && \
	$(UNLZMA) < $(abs_srcdir)/$(INFORMTARBALL) | $(TAR) $(EXCLUDE_ARGS) -t \
		> uninstall_manifest.txt && \
	$(UNLZMA) < $(abs_srcdir)/$(INFORMTARBALL) | $(TAR) $(EXCLUDE_ARGS) --no-same-owner -x && \
	$(SED) -e 's/^\|$$/"/g' < uninstall_manifest.txt \
		| $(XARGS) $(CHMOD) a=rX,u+w

uninstall-local:
	cd $(DESTDIR)$(pkgdatadir) && \
	$(SED) -e 's/ /\\ /g' < uninstall_manifest.txt | $(XARGS) rm -f; \
	rm -f $(DESTDIR)$(pkgdatadir)/uninstall_manifest.txt

-include $(top_srcdir)/git.mk
