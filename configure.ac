### configure.ac with sensible comments ########################################

### INITIALIZATION #############################################################

# Initialize autoconf
# Args: (human-readable package name, package version, bug report address,
# tarballname)
AC_INIT([Inform 7], [6M62], [http://inform7.com/mantis], [gnome-inform7])
# Require Autoconf 2.64; I think the newest feature is m4_map_args_w...
AC_PREREQ([2.64])
# Sanity check to make sure we are running Autoconf from the right directory
AC_CONFIG_SRCDIR(src/main.c)
# Tell where to put the libtool macros
AC_CONFIG_MACRO_DIR([m4])
# Initialize automake; pkglibexecdir and silent rules introduced in 1.11
# foreign: Don't require GNU docs such as NEWS
# subdir-objects: forwards compatibility with 2.0
# dist-xz/no-dist-gzip: distribute .tar.xz instead of .tar.gz
AM_INIT_AUTOMAKE([1.11 -Wall foreign subdir-objects dist-xz no-dist-gzip])
# Configure with --enable-silent-rules to cut down on clutter
AM_SILENT_RULES([yes])
# Allow "AM_GST_ELEMENT_CHECK" for building without GStreamer development files
m4_pattern_allow([^AM_GST_ELEMENT_CHECK$])

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
AC_PATH_PROG([EVINCE], [evince], [notfound])
# Check for pdftex and evince even though they are not required; in any case it
# will fill in the proper paths in inweb-configuration.txt.
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
		[Configure for make distcheck @<:@default=no@:>@])])
AM_CONDITIONAL([DISTCHECK], [$TEST "x$enable_distcheck" = xyes])

# --enable-manuals builds the inweb and cBlorb manuals in HTML format. PDF is
# also possible, but requires more build dependencies, so you'll have to do that
# manually.
AC_ARG_ENABLE([manuals],
	[AS_HELP_STRING([--enable-manuals=@<:@yes/sections/no@:>@],
		[Build the inweb and cBlorb manuals @<:@default=no@:>@])],
	[],
	[enable_manuals=no])
AM_CONDITIONAL([MANUALS], [$TEST "x$enable_manuals" != xno])
AM_CONDITIONAL([MANUALS_SECTIONS], [$TEST "x$enable_manuals" = xsections])

### DECLARE PROGRAMS ##########################################################

# Programs that Autoconf has built-in checks for
AC_PROG_CC                        # C compiler
AC_PROG_CC_C99                    # ...and put it in C99 mode
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
PKG_PROG_PKG_CONFIG               # pkg_config
GLIB_GSETTINGS                    # used for GSettings

AC_PATH_PROG([UNLZMA], [unlzma], [notfound]) # nonGNU tar doesn't have --lzma
AS_IF([$TEST "x$UNLZMA" = xnotfound],
	[AC_MSG_ERROR([The unlzma program is required to install Inform 7.
	Try installing it from your package manager.])])

# ITS tool not needed for tarball build, so only error out if a git clone
AC_PATH_PROG([ITSTOOL], [itstool], [itstool-notfound])
AS_IF([$TEST "x$ITSTOOL" = xitstool-notfound -a -d .git],
	[AC_MSG_ERROR([ITS Tool is needed to compile Inform 7 from a Git checkout.
	Try installing it from your package manager.])])

### INTERNATIONALIZATION #######################################################

AM_GNU_GETTEXT_VERSION([0.19.2])
# 0.18.3 required for Glade files; but use last point release from supported
# distros (currently openSUSE 13.2 has this version, Debian stable has .3)
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

# Decide which sound library to use
AC_ARG_WITH([sound],
	[AS_HELP_STRING([--with-sound=@<:@no/gstreamer@:>@],
		[Choose library with which to implement Glk sound])],
	[],
	[with_sound=gstreamer])

AS_IF([test "x$with_sound" != "xno"],
	[AC_DEFINE([HAVE_SOUND], [1], [Define if any sound support is enabled])])

# Libraries for main program
PKG_CHECK_MODULES([INFORM7], [
	gdk-3.0
	glib-2.0 >= 2.32
	gtk+-3.0
	gio-2.0 >= 2.30
	gtksourceview-3.0
	webkit2gtk-4.0
	gtkspell3-3.0
	libxml-2.0
	goocanvas-2.0
	$UUID_MODULE
])
PKG_CHECK_MODULES([OSXCART], [
	glib-2.0 >= 2.18
	gio-2.0 >= 2.18
	gdk-3.0
	gdk-pixbuf-2.0 >= 2.6
	gtk+-3.0
])
PKG_CHECK_MODULES([CHIMARA], [
	glib-2.0 >= 2.40
	gtk+-3.0 >= 3.10
	gthread-2.0
	gmodule-2.0
	pango
	gstreamer-1.0
])

# GStreamer plugins needed to run library
AS_CASE([$with_sound],
	[gstreamer], [
		have_all_elements=yes
		m4_ifdef([GST_ELEMENT_CHECK], [
			m4_map_args_w([
				giostreamsrc
				typefind
				audioconvert
				volume
				oggdemux
				vorbisdec
				autoaudiosink
				aiffparse
				modplug],
				[GST_ELEMENT_CHECK(], [, [1.0], [], [have_all_elements=no])])
			AS_IF([test "x$have_all_elements" = "xno"],
				[AC_MSG_ERROR([One or more required GStreamer elements were not found.
You will have to install your system's "base", "good", and "bad" plugins
packages. Try looking for names such as: gstreamer1-plugins-base,
gstreamer1-plugins-good, gstreamer1-plugins-bad-free,
gstreamer1-plugins-bad-free-extras, gstreamer1.0-plugins-base,
gstreamer1.0-plugins-good, gstreamer1.0-plugins-bad])])],
			[AC_MSG_ERROR([GST_ELEMENT_CHECK not found. Install the development package
for GStreamer 1.0 and rerun autogen.sh.])])
		AC_DEFINE([GSTREAMER_1_0_SOUND], [1],
			[Define to enable sound support with GStreamer 1.0])
	],
	[])

### WHETHER TO RUN TESTS IN VIRTUAL X SERVER ###################################
# Requires xvfb
AC_ARG_WITH([xvfb-tests],
       AS_HELP_STRING([--with-xvfb-tests],
               [Use xvfb to run tests, e.g. on build servers @<:@default=no@:>@]))
AS_IF([test "x$with_xvfb_tests" != "xno"],
       [AC_PATH_PROG([XVFB], [xvfb-run], [notfound])])
AS_IF([test "x$XVFB" != "xnotfound"],
       [AC_SUBST([XVFB_RUN], ["$XVFB "])],  dnl can add other xvfb-run args here
       [AS_IF([test "x$with_vfb_tests" = "yes"],
               [AC_MSG_ERROR([XVFB tests requested, but xvfb-run not found])])])

### DEFINE OTHER VARIABLES #####################################################

# Inform 6 version that goes along with this release
AC_DEFINE_UNQUOTED([INFORM6_COMPILER_VERSION],
	["6.33"], [Version of Inform 6 compiler.])

# Plugin flags; include '-module' in each Makefile.am, because AC_SUBSTed
# variables are black boxes to Automake, so it has to know about it being a
# module in the makefile itself.
PLUGIN_LIBTOOL_FLAGS='-avoid-version -shared -export-symbols-regex "^glk"'
AC_SUBST(PLUGIN_LIBTOOL_FLAGS)

# Get current year for updating copyrights
AC_SUBST(COPYRIGHT_YEAR, [`$DATE +%Y`])
AC_DEFINE_UNQUOTED([COPYRIGHT_YEAR], ["$COPYRIGHT_YEAR"], [Year this program was built.])

# Compiler flags
# Justification for -Wno-unused-parameter: GLib signal handling code has many
# callback types with certain signatures, and more often than not we don't need
# to use all the parameters passed to a callback.
AX_APPEND_COMPILE_FLAGS([-Wall -Wextra -Wno-unused-parameter], [WARNINGFLAGS])
AC_SUBST([WARNINGFLAGS])
AX_APPEND_COMPILE_FLAGS([-Wno-unused-result], [INWEB_EXTRAFLAGS])
AC_SUBST([INWEB_EXTRAFLAGS])
AX_APPEND_COMPILE_FLAGS([-Wno-pointer-to-int-cast], [INFORM6_EXTRAFLAGS])
AC_SUBST([INFORM6_EXTRAFLAGS])
AX_APPEND_COMPILE_FLAGS([-Wno-tautological-compare], [CHIMARA_EXTRAFLAGS])
AC_SUBST([CHIMARA_EXTRAFLAGS])
AX_APPEND_COMPILE_FLAGS([-Wno-implicit-function-declaration],
	[FROTZ_EXTRAFLAGS])
AC_SUBST([FROTZ_EXTRAFLAGS])
AX_APPEND_COMPILE_FLAGS([-Wno-pointer-sign], [GIT_EXTRAFLAGS])
AC_SUBST([GIT_EXTRAFLAGS])

### OUTPUT #####################################################################

# Output platform-specific definitions to config.h
AC_CONFIG_HEADERS([config.h])
# List of other files for Autoconf to output
AC_CONFIG_FILES([
Makefile
src/Makefile
src/inform6/Makefile
src/inlib/Makefile
src/inweb/Makefile
src/inweb/Materials/inweb-configuration.txt
src/cBlorb/Makefile
src/ni/Makefile
po/Makefile.in
data/Makefile
data/ui/Makefile
data/licenses/Makefile
data/licenses/license.html
data/highlighting/Makefile
data/mime/Makefile
data/styles/Makefile
pixmaps/Makefile
])
# Do it
AC_OUTPUT
