compilersdir = $(pkgdatadir)/Compilers
compilers_DATA = cBlorb inform-6.31-biplatform ni
#
# UNCOMMENT the following line when building RPMs!
# 
EXTRA_DIST = $(compilers_DATA)

ni:
	if [ ! -f cBlorb ] || [ ! -f inform-6.31-biplatform ] || [ ! -f ni ]; \
	then { clear; \
	echo "IMPORTANT NOTE! You must download the appropriate compiler package"; \
	echo "from www.inform-fiction.org and put the following files:"; \
	echo "  ni"; \
	echo "  inform-6.31-biplatform"; \
	echo "  cBlorb"; \
	echo "into the directory"; pwd; exit 1; } \
	fi

cBlorb: ni

inform-6.31-biplatform: ni

install-data-hook:
	pushd "$(DESTDIR)$(compilersdir)"; \
	chmod a+x cBlorb inform-6.31-biplatform ni; \
	popd
