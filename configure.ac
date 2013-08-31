### configure.ac with sensible comments ########################################

### INITIALIZATION #############################################################

# Initialize autoconf
# Args: (human-readable package name, package version, bug report address,
# tarballname)
AC_INIT([Inform 7], [6G60], [http://inform7.com/mantis], [gnome-inform7])
# Require Autoconf 2.60; I think the newest feature is AC_PROG_MKDIR_P...
AC_PREREQ([2.60])
# Sanity check to make sure we are running Autoconf from the right directory
AC_CONFIG_SRCDIR(src/main.c)
# Tell where to put the libtool macros
AC_CONFIG_MACRO_DIR([m4])
# Initialize automake; $(MKDIR_P) was introduced in 1.10
AM_INIT_AUTOMAKE([1.10 -Wall])
# Configure with --enable-silent-rules to cut down on clutter
# Requires automake 1.11
m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES([yes])])

### CHECK FOR PROPER NI EXECUTABLE #############################################

# Grep is required for these tests.
AC_PROG_GREP
AC_PROG_EGREP

AC_CHECK_FILE([${srcdir}/src/ni/ni], [NI=${srcdir}/src/ni/ni], [
	AC_MSG_ERROR([ni not found in ${srcdir}/src/ni.
	IMPORTANT NOTE! You must download version $VERSION of the appropriate
	compiler package from inform7.org and put the file `ni' into the
	directory `${srcdir}/src/ni'.
	(Try http://inform7.com/download/content/$VERSION/I7_${VERSION}_Linux_all.tar.gz)])
])
AC_MSG_CHECKING([the version of ${NI}])
AS_IF([${NI} | $GREP $VERSION > /dev/null], [AC_MSG_RESULT([$VERSION])], [
	AC_MSG_ERROR([You have version
	[$(${NI} | $EGREP -o '[0-9][A-Z][0-9]{2}')] of ni in src/ni. This version of Inform 7 will only work with
	version $VERSION.
	IMPORTANT NOTE! You must download version $VERSION of the appropriate
	compiler package from inform7.com and put the file `ni' into the
	directory `${srcdir}/src/ni'.
	(Try http://inform7.com/download/content/$VERSION/I7_${VERSION}_Linux_all.tar.gz)])
])

### DECLARE OPTIONS ############################################################

# Test is required for these option checks
AC_PATH_PROG([TEST], [test])
# Optional programs
AC_PATH_PROG([PDFTEX], [pdftex], [notfound])
AC_PATH_PROG([DOT], [dot], [notfound])
AC_PATH_PROG([RPMBUILD], [rpmbuild], [notfound])
AC_PATH_PROG([RPM], [rpm], [notfound])
AC_PATH_PROG([DEBUILD], [dpkg-buildpackage], [notfound])

# --enable-rpm copies the ni executable and the spec file to the dist package,
# for use in building RPM packages. Requires rpm and rpmbuild.
AC_ARG_ENABLE([rpm],
	[AS_HELP_STRING([--enable-rpm=@<:@yes/no@:>@],
		[Configure for building RPM package @<:@default=no@:>@ (requires rpm and rpmbuild)])],
	[AS_IF([$TEST "x$enable_rpm" = xyes],
		[AS_IF([$TEST "x$RPMBUILD" = xnotfound],
			[AC_MSG_ERROR([rpmbuild is required for --enable-rpm])],
		[$TEST "x$RPM" = xnotfound],
			[AC_MSG_ERROR([rpm is required for --enable-rpm])])])],
	[enable_rpm=no])
AM_CONDITIONAL(BUILDING_RPM, $TEST "x$enable_rpm" = xyes)

# --enable-deb copies only the ni executable to the dist package, for use in
# building DEB packages.
AC_ARG_ENABLE([deb],
	[AS_HELP_STRING([--enable-deb=@<:@yes/no@:>@],
		[Configure for building Debian package @<:@default=no@:>@ (requires dpkg-buildpackage)])],
	[AS_IF([$TEST "x$enable_deb" = xyes],
		[AS_IF([$TEST "x$DEBUILD" = xnotfound],
			[AC_MSG_ERROR([dpkg-buildpackage is required for --enable-deb])],
		[$TEST "x$enable_rpm" = xyes],
			[AC_MSG_ERROR([--enable-rpm and --enable-deb are mutually exclusive])])])],
	[enable_deb=no])
AM_CONDITIONAL([BUILDING_DEB], [$TEST "x$enable_deb" = xyes])

# --enable-distcheck does the same thing as --enable-deb, but doesn't require
# dpkg-buildpackage. This is because the tarball for distcheck needs to have
# the ni binary in it, otherwise configuring it will fail.
AC_ARG_ENABLE([distcheck],
	[AS_HELP_STRING([--enable-distcheck=@<:@yes/no@:>@],
		[Configure for make distcheck @<:@default=no@:>@])],
	[]
	[enable_distcheck=no])
AM_CONDITIONAL([DISTCHECK], [$TEST "x$enable_distcheck" = xyes])

# --enable-manuals builds the inweb and cBlorb manuals. Requires pdftex and dot.
AC_ARG_ENABLE([manuals],
	[AS_HELP_STRING([--enable-manuals=@<:@yes/chapters/sections/no@:>@],
		[Build the inweb and cBlorb manuals @<:@default=no@:>@ (requires pdftex and dot)])],
	[AS_IF([$TEST "x$enable_manuals" != xno],
		[AS_IF([$TEST "x$PDFTEX" = xnotfound],
			[AC_MSG_ERROR([pdftex is required for --enable-manuals])],
		[$TEST "x$DOT" = xnotfound],
			[AC_MSG_ERROR([dot is required for --enable-manuals])])])],
	[enable_manuals=no])
AM_CONDITIONAL([MANUALS], [$TEST "x$enable_manuals" != xno])
AM_CONDITIONAL([MANUALS_CHAPTERS], [$TEST "x$enable_manuals" = xchapters])
AM_CONDITIONAL([MANUALS_SECTIONS], [$TEST "x$enable_manuals" = xsections])

# The user can specify the RPM release number in the environment (e.g. '1.fc10')
AC_ARG_VAR([RPM_RELEASE], [Release number to build RPMs with [1]])
AS_IF([$TEST "x$RPM_RELEASE" = x], [RPM_RELEASE=1])

### DECLARE PROGRAMS ##########################################################

# Programs that Autoconf has built-in checks for
AC_PROG_CC                        # C compiler
AM_PROG_CC_C_O                    # Automake requires this for per-target CFLAGS
AC_PROG_INSTALL                   # Install
AC_PROG_SED                       # Sed
AC_PROG_MKDIR_P                   # mkdir -p
AM_PROG_AR                        # ar
LT_INIT([dlopen])                 # Libtool 2.2.6 and up
LT_PREREQ([2.2.6])

# Other tools used in this package's various make scripts
AC_PATH_PROG([PERL], [perl])      # Perl
AC_PATH_PROG([TAR], [tar gtar])   # tar
AC_PATH_PROG([XARGS], [xargs])    # xargs
AC_PATH_PROG([CHMOD], [chmod])    # chmod
AC_PATH_PROG([CAT], [cat])        # cat
AC_PATH_PROG([ECHO], [echo])      # echo
AC_PATH_PROG([CP], [cp])          # cp
AC_PATH_PROG([DATE], [date])      # date
AC_PATH_PROG([MV], [mv])          # mv
# Other programs
AM_GNU_GETTEXT([external])        # Gettext, link to system libintl
IT_PROG_INTLTOOL([0.35.0])        # Intltool
PKG_PROG_PKG_CONFIG               # pkg_config
AM_GCONF_SOURCE_2                 # GConf
AC_PATH_PROG([GCONFTOOL], [gconftool-2]) # GConf-Tool

AC_PATH_PROG([UNLZMA], [unlzma], [notfound]) # nonGNU tar doesn't have --lzma
AS_IF([$TEST "x$UNLZMA" = xnotfound],
	[AC_MSG_ERROR([The unlzma program is required to install Inform 7.
	Try installing it from your package manager.])])

### INTERNATIONALIZATION #######################################################

AM_GNU_GETTEXT_VERSION([0.17])
GETTEXT_PACKAGE=gnome-inform7
AC_SUBST(GETTEXT_PACKAGE)
AC_DEFINE_UNQUOTED(GETTEXT_PACKAGE,"$GETTEXT_PACKAGE", [Gettext package.])

### CHECK FOR LIBRARIES ########################################################

# Decide which UUID library to use
# Prefer e2fsprogs, and switch to OSSP if that doesn't exist
PKG_CHECK_EXISTS([uuid],
	[AC_DEFINE([E2FS_UUID], [1], [Define if uuid header is from e2fsprogs])
		UUID_MODULE=uuid],
	[PKG_CHECK_EXISTS([ossp-uuid],
		[UUID_MODULE=ossp-uuid],
		[AC_MSG_ERROR([Either OSSP UUID or e2fsprogs UUID required.])]
	)]
)
# Also, check which header is used
AC_CHECK_HEADERS([uuid.h ossp/uuid.h uuid/uuid.h])

# If webkit-1.0 is a later version than 1.6.0, then we need to pull in
# javascriptcoregtk-1.0 as well
PKG_CHECK_EXISTS([webkit-1.0 >= 1.6.0],
	[JAVASCRIPTCORE_MODULE=javascriptcoregtk-1.0],
	[JAVASCRIPTCORE_MODULE=]
)

# Libraries for main program
PKG_CHECK_MODULES([INFORM7], [
	gdk-2.0 >= 2.2
	glib-2.0 >= 2.16
	gtk+-2.0 >= 2.20
	gio-2.0 >= 2.18
	gconf-2.0 >= 2.4
	gtksourceview-2.0 >= 2.2
	webkit-1.0 >= 1.0
	$JAVASCRIPTCORE_MODULE
	gtkspell-2.0
	libxml-2.0
	goocanvas
	$UUID_MODULE
])
PKG_CHECK_MODULES([OSXCART], [
	glib-2.0 >= 2.16
	gdk-2.0
	gdk-pixbuf-2.0 >= 2.6
	gtk+-2.0 >= 2.10
])
PKG_CHECK_MODULES([CHIMARA], [
	glib-2.0 >= 2.16
	gtk+-2.0 >= 2.12
	gthread-2.0
	gmodule-2.0
	pango
	gstreamer-0.10 >= 0.10.12
])

AC_DEFINE([GSTREAMER_SOUND], [1], [Define to enable sound support with GStreamer])

# GStreamer plugins needed to run library
m4_defun([AX_GST_REQUIRE_ELEMENT],
	[AM_GST_ELEMENT_CHECK([$1],
		[],
		[AC_MSG_ERROR([GStreamer element $1 not found. You need to install gstreamer-plugins-m4_default([$2], [base]).])]
	)]
)
AX_GST_REQUIRE_ELEMENT([giostreamsrc])
AX_GST_REQUIRE_ELEMENT([typefind])
AX_GST_REQUIRE_ELEMENT([audioconvert])
AX_GST_REQUIRE_ELEMENT([volume])
AX_GST_REQUIRE_ELEMENT([oggdemux])
AX_GST_REQUIRE_ELEMENT([vorbisdec])
AX_GST_REQUIRE_ELEMENT([autoaudiosink], [good])
AX_GST_REQUIRE_ELEMENT([aiffparse], [bad])
AX_GST_REQUIRE_ELEMENT([modplug], [bad])

# Plugin flags; include '-module' in each Makefile.am, because AC_SUBSTed
# variables are black boxes to Automake, so it has to know about it being a
# module in the makefile itself.
PLUGIN_LIBTOOL_FLAGS='-avoid-version -shared -export-symbols-regex "^glk"'
AC_SUBST(PLUGIN_LIBTOOL_FLAGS)

# Get current year for updating copyrights
AC_SUBST(COPYRIGHT_YEAR, [`$DATE +%Y`])
AC_DEFINE_UNQUOTED([COPYRIGHT_YEAR], ["$COPYRIGHT_YEAR"], [Year this program was built.])

### OUTPUT #####################################################################

# Output platform-specific definitions to config.h
AC_CONFIG_HEADERS([config.h])
# List of other files for Autoconf to output
AC_CONFIG_FILES([
Makefile
gnome-inform7.spec
src/Makefile
src/inform6/Makefile
src/inweb/Makefile
src/inweb/Materials/inweb-configuration.txt
src/cBlorb/Makefile
src/ni/Makefile
po/Makefile.in
data/Makefile
data/ui/Makefile
data/licenses/Makefile
data/licenses/license.html
data/languages/Makefile
data/styles/Makefile
pixmaps/Makefile
])
# Do it
AC_OUTPUT
