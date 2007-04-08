compilersdir = $(pkgdatadir)/Compilers
compilers_DATA = cBlorb inform-6.31-biplatform ni
EXTRA_DIST = $(compilers_DATA)

install-data-hook:
	pushd "$(compilersdir)"; \
	chmod a+x cBlorb inform-6.31-biplatform ni; \
	popd
