#!/bin/sh
### autogen.sh with sensible comments ###############################

# Use this script to bootstrap your build AFTER checking it out from
# source control. You should not have to use it for anything else.

### PATCHING EXTERNALS ##############################################
# Make local modifications to externals checked out from other
# repositories. Skip this step if the patches have already been
# applied.
echo "Patching externals"
patch -N -r - src/osxcart/Makefile.am src/osxcart.Makefile.am.patch
cp src/interpreters/frotz.Makefile.am src/interpreters/frotz/Makefile.am
cp src/interpreters/glulxe.Makefile.am src/interpreters/glulxe/Makefile.am
cp src/interpreters/git.Makefile.am src/interpreters/git/Makefile.am
rm -f \
	src/interpreters/glulxe/Makefile \
	src/interpreters/glulxe/glulxdump.c \
	src/interpreters/glulxe/macstart.c \
	src/interpreters/glulxe/profile-analyze.py \
	src/interpreters/glulxe/winstart.c \
	src/interpreters/git/Makefile \
	src/interpreters/git/Makefile.win \
	src/interpreters/git/git_mac.c \
	src/interpreters/git/git_windows.c 
rm -rf \
	src/interpreters/git/test \
	src/interpreters/git/win

### AUTOTOOLS #######################################################
# Runs autoconf, autoheader, aclocal, automake, autopoint, libtoolize
echo "Regenerating autotools files"
autoreconf --install --force --symlink || exit 1

### INTLTOOL ########################################################
# Run after autopoint or glib-gettextize
echo "Setting up intltool"
intltoolize --copy --force --automake || exit 1

echo "Now run ./configure, make, and make install."
