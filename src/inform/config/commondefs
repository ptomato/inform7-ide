# Common makefile targets, in -*- Makefile -*- mode.

# Version-specific installation directory.
installdir = $(pkgdatadir)/$(INFORM_VERSION)

# Place to put include files.
includedir = $(installdir)/include

# Place to put modules and headers.
moduledir = $(installdir)/module

# Place for users to put local include files.
localincdir = $(pkgdatadir)/include

# Place for users to put local modules and headers.
localmoddir = $(pkgdatadir)/module

# Etags pattern to match Inform tags.
TYPES = \(Class\|Object\|Nearby\)
ETAGS_INFORM = -l none -r '/$(TYPES)\([ \t]+->\)*[ \t]+\([^ \t]+\)/\3/' \
-r '/\[[ \t]+\([^ \t;]+\)/\1/'
