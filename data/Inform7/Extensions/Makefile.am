SUBDIRS = EmilyShort GrahamNelson Reserved

install-data-hook:
	pushd "$(DESTDIR)$(pkgdatadir)/Inform7/Extensions"; \
	ln -s Emily\ Short emily\ short; \
	ln -s Graham\ Nelson graham\ nelson; \
	popd

uninstall-hook:
	pushd "$(DESTDIR)$(pkgdatadir)/Inform7/Extensions"; \
	rm -f emily\ short graham\ nelson; \
	popd
