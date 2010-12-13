SUBDIRS = . licenses languages styles ui

INFORMTARBALL = Inform_@PACKAGE_VERSION@_data.tar.lz

dist_pkgdata_DATA = $(INFORMTARBALL)

install-data-hook:
	cd $(DESTDIR)$(pkgdatadir); \
	$(CAT) $(INFORMTARBALL) | $(UNLZMA) | $(TAR) --list > uninstall_manifest.txt; \
	$(CAT) $(INFORMTARBALL) | $(UNLZMA) | $(TAR) --get; \
	rm --force $(INFORMTARBALL); \
	$(CAT) uninstall_manifest.txt | $(SED) --expression='s/^\|$$/"/g' \
		| $(XARGS) $(CHMOD) a=rX,u+w

uninstall-local:
	cd $(DESTDIR)$(pkgdatadir); \
	$(CAT) uninstall_manifest.txt | $(SED) --expression='s/ /\\ /' \
		| $(XARGS) rm --force; \
	rm --force uninstall_manifest.txt

