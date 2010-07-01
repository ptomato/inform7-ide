### configure.ac with sensible comments ########################################

### INITIALIZATION #############################################################

# Initialize autoconf
# Args: (human-readable package name, package version, bug report address, 
# tarballname)
AC_INIT([GNOME Inform 7], [6E59], [http://inform7.com/contribute/report], [gnome-inform7])
# Require Autoconf 2.60; I think the newest feature is AC_PROG_MKDIR_P...
AC_PREREQ([2.60])
# Sanity check to make sure we are running Autoconf from the right directory
AC_CONFIG_SRCDIR(src/main.c)
# Initialize automake; $(MKDIR_P) was introduced in 1.10
AM_INIT_AUTOMAKE([1.10 -Wall])

### CHECK FOR PROPER NI EXECUTABLE #############################################

# Grep is required for these tests.
AC_PROG_GREP
AC_PROG_EGREP

AC_CHECK_FILE([src/ni/ni], [], [
	AC_MSG_ERROR([ni not found in src/ni.
	IMPORTANT NOTE! You must download version $VERSION of the appropriate 
	compiler package from inform7.org and put the file `ni' into the 
	directory `$(pwd)/src/ni'.])
])
AC_MSG_CHECKING([the version of src/ni/ni])
AS_IF([src/ni/ni | $GREP $VERSION > /dev/null], [AC_MSG_RESULT([$VERSION])], [
	AC_MSG_ERROR([You have version 
	[$(src/ni/ni | $EGREP -o '[0-9][A-Z][0-9]{2}')] of ni in src/ni. This version of Inform 7 will only work with
	version $VERSION.
	IMPORTANT NOTE! You must download version $VERSION of the appropriate 
	compiler package from inform7.org and put the file `ni' into the 
	directory `$(pwd)/src/ni'.])
])

### DECLARE OPTIONS ############################################################

# Test is required for these option checks
AC_PATH_PROG([TEST], [test])
# Optional programs
AC_PATH_PROG([PDFTEX], [pdftex], [notfound])
AC_PATH_PROG([DOT], [dot], [notfound])
AC_PATH_PROG([RPMBUILD], [rpmbuild], [notfound])
AC_PATH_PROG([RPM], [rpm], [notfound])
AC_PATH_PROG([DEBUILD], [debuild], [notfound])

# --enable-rpm copies the ni executable and the spec file to the dist package,
# for use in building RPM packages. Requires rpm and rpmbuild.
AC_ARG_ENABLE([rpm],
	[AS_HELP_STRING([--enable-rpm],[Configure for building RPM package [no] (requires rpm and rpmbuild)])],
	[AS_CASE([${enableval}],
		[yes], [AS_IF(
			[$TEST "x$RPMBUILD" = xnotfound],
				[AC_MSG_ERROR([rpmbuild is required for --enable-rpm])],
			[$TEST "x$RPM" = xnotfound],
				[AC_MSG_ERROR([rpm is required for --enable-rpm])],
			[rpm=true])],
		[no],  [rpm=false],
		[AC_MSG_ERROR([bad value ${enableval} for --enable-rpm])]
	)],
	[rpm=false]
)
AM_CONDITIONAL([BUILDING_RPM], [$TEST "x$rpm" = xtrue])

# --enable-deb copies only the ni executable to the dist package, for use in
# building DEB packages.
AC_ARG_ENABLE([deb],
	[AS_HELP_STRING([--enable-deb],[Configure for building DEB package [no]])],
	[AS_CASE([${enableval}],
		[yes], [AS_IF(
			[$TEST "x$DEBUILD" = xnotfound],
				[AC_MSG_ERROR([debuild is required for --enable-deb])],
			[$TEST "x$rpm" = xtrue],
				[AC_MSG_ERROR([--enable-rpm and --enable-deb are mutually exclusive])],
			[deb=true])],
		[no],  [deb=false],
		[AC_MSG_ERROR([bad value ${enableval} for --enable-deb])]
	)],
	[deb=false]
)
AM_CONDITIONAL([BUILDING_DEB], [$TEST "x$deb" = xtrue])

# --enable-manuals builds the inweb and cBlorb manuals. Requires pdftex and dot.
AC_ARG_ENABLE([manuals],
	[AS_HELP_STRING([--enable-manuals], [Build the inweb and cBlorb manuals [no] (requires pdftex and dot; optionally specify 'chapters' or 'sections')])],
	[AS_CASE([${enableval}],
		[yes|chapters|sections], [AS_IF(
			[$TEST "x$PDFTEX" = xnotfound],
				[AC_MSG_ERROR([pdftex is required for --enable-manuals])],
			[$TEST "x$DOT" = xnotfound],
				[AC_MSG_ERROR([dot is required for --enable-manuals])],
			[manuals=true])
			AS_IF(
				[$TEST "x${enableval}" = xsections], [manualtype=sections],
				[$TEST "x${enableval}" = xchapters], [manualtype=chapters],
				[manualtype=justthefacts]
			)],
		[no], [manuals=false],
		[AC_MSG_ERROR([bad value ${enableval} for --enable-manuals])]
	)],
	[manuals=false]
)
AM_CONDITIONAL([MANUALS], [$TEST "x$manuals" = xtrue])
AM_CONDITIONAL([MANUALS_CHAPTERS], [$TEST "x$manualtype" = xchapters])
AM_CONDITIONAL([MANUALS_SECTIONS], [$TEST "x$manualtype" = xsections])

# The user can specify the RPM release number in the environment (e.g. '1.fc10')
AC_ARG_VAR([RPM_RELEASE], [Release number to build RPMs with [1]])
AS_IF([test "x$RPM_RELEASE" = x], [RPM_RELEASE=1])

### DECLARE PROGRAMS ###########################################################

# Programs that Autoconf has built-in checks for
AC_PROG_CC                        # C compiler
AM_PROG_CC_C_O                    # Automake requires this for per-target CFLAGS
AC_PROG_INSTALL                   # Install
AC_PROG_RANLIB                    # Ranlib
AC_PROG_SED                       # Sed
AC_PROG_MKDIR_P                   # mkdir -p
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
AM_GLIB_GNU_GETTEXT               # Use GLib gettext instead of standard
IT_PROG_INTLTOOL([0.35.0])        # Intltool
PKG_PROG_PKG_CONFIG               # pkg_config
AM_GCONF_SOURCE_2                 # GConf
AC_PATH_PROG([GCONFTOOL], [gconftool-2]) # GConf-Tool

### INTERNATIONALIZATION #######################################################

GETTEXT_PACKAGE=gnome-inform7
AC_SUBST(GETTEXT_PACKAGE)
AC_DEFINE_UNQUOTED(GETTEXT_PACKAGE,"$GETTEXT_PACKAGE", [Gettext package.])

### CHECK FOR LIBRARIES ########################################################

# Check whether to use GtkHTML 3.14 or 3.8
PKG_CHECK_EXISTS([libgtkhtml-3.14], 
	[GTK_HTML_MODULE=libgtkhtml-3.14], 
	[GTK_HTML_MODULE=libgtkhtml-3.8])
# Check for the correct UUID library header location
# Prefer e2fsprogs, and switch to OSSP if that doesn't exist
PKG_CHECK_EXISTS([uuid],
	[AC_DEFINE([E2FS_UUID], [1], [Define if uuid header is from e2fsprogs])
		UUID_MODULE=uuid],
	[PKG_CHECK_EXISTS([ossp-uuid],
		[AC_CHECK_HEADER([ossp/uuid.h],
			[AC_DEFINE([OSSP_UUID], [1], 
				[Define if the UUID header is not <uuid.h> but <ossp/uuid.h>])])
		UUID_MODULE=ossp-uuid],
		[AC_MSG_ERROR([Either OSSP UUID or e2fsprogs UUID required.])]
	)]
)	
# Libraries for main program
PKG_CHECK_MODULES([GNOMEINFORM7], [
	glib-2.0 >= 2.4.0
	gdk-2.0
	gdk-pixbuf-2.0
	gtk+-2.0 >= 2.8.0
	pango
	libgnome-2.0
	libgnomeui-2.0
	gconf-2.0
	gnome-vfs-2.0
	libbonobo-2.0
	gtksourceview-2.0
	gtkspell-2.0
	libxml-2.0
	$GTK_HTML_MODULE
	$UUID_MODULE
])

# Libraries for GtkTerp
PKG_CHECK_MODULES([GTKTERP], [
	gdk-2.0
	gtk+-2.0 >= 2.8.0
	freetype2 
	sdl
	libpng 
	dbus-glib-1
])
# SDL mixer, SDL sound and libjpeg don't use pkg-config
# Also, -lm needs to be added here to work around Fedora bug FIXME
GTKTERP_LIBS="$GTKTERP_LIBS -lSDL_mixer -lSDL_sound -ljpeg -lm"
AC_SUBST(GTKTERP_LIBS)

# Get current year for updating copyrights
AC_SUBST(COPYRIGHT_YEAR, [`$DATE +%Y`])

### OUTPUT #####################################################################

# Output platform-specific definitions to config.h
AC_CONFIG_HEADERS([config.h])
# List of other files for Autoconf to output
AC_CONFIG_FILES([
Makefile
gnome-inform7.spec
src/Makefile
src/interface.c
src/gtkterp/Makefile
src/gtkterp/frotz/Makefile
src/gtkterp/glulxe/Makefile
src/gtkterp/garglk/Makefile
src/gtkterp/git/Makefile
src/inform6/Makefile
src/inweb/Makefile
src/inweb/Materials/inweb-configuration.txt
src/cBlorb/Makefile
src/ni/Makefile
po/Makefile.in
data/Makefile
data/licenses/Makefile
data/languages/Makefile
data/styles/Makefile
pixmaps/Makefile
])
# Do it
AC_OUTPUT

