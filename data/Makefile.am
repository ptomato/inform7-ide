SUBDIRS = . licenses languages styles ui

INFORMTARBALL = Inform_@PACKAGE_VERSION@_data.tar.lz

EXTRA_DIST = $(INFORMTARBALL)

install-data-hook:
	cd $(DESTDIR)$(pkgdatadir); \
	$(UNLZMA) < $(srcdir)/$(INFORMTARBALL) | $(TAR) -t > uninstall_manifest.txt; \
	$(UNLZMA) < $(srcdir)/$(INFORMTARBALL) | $(TAR) --no-same-owner -x; \
	$(SED) -e 's/^\|$$/"/g' < uninstall_manifest.txt \
		| $(XARGS) $(CHMOD) a=rX,u+w

uninstall-local:
	cd $(DESTDIR)$(pkgdatadir); \
	$(SED) -e 's/ /\\ /g' < uninstall_manifest.txt | $(XARGS) rm -f; \
	rm -f uninstall_manifest.txt

-include $(top_srcdir)/git.mk
