interpretersdir = $(pkgdatadir)/Interpreters
interpreters_DATA = dumb-glulxe
EXTRA_DIST = $(interpreters_DATA)

install-data-hook:
	pushd "$(interpretersdir)"; \
	chmod a+x dumb-glulxe; \
	popd
