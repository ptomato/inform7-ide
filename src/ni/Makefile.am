# This line won't be necessary after Automake 1.10a
pkglibexecdir = $(libexecdir)/$(PACKAGE)

# We call it a "script" so that it will get installed as an executable,
# but won't get cleaned
pkglibexec_SCRIPTS = ni

# We leave it out of the regular source distribution produced by "make dist",
# but leave it in when we're feeding the dist tarball to rpmbuild or debuild
if BUILDING_RPM
EXTRA_DIST = ni
endif
if BUILDING_DEB
EXTRA_DIST = ni
endif
