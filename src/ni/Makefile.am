# We call it a "script" so that it will get installed as an executable,
# but won't get cleaned
pkglibexec_SCRIPTS = ni

# We leave it out of the regular source distribution produced by "make dist",
# but leave it in when we're feeding the dist tarball to rpmbuild or debuild
# or making distcheck
if BUILDING_RPM
EXTRA_DIST = ni
endif
if BUILDING_DEB
EXTRA_DIST = ni
endif
if DISTCHECK
EXTRA_DIST = ni
endif

GITIGNOREFILES = ni

-include $(top_srcdir)/git.mk
