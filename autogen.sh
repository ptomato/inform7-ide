#!/bin/sh
### autogen.sh with sensible comments ###############################

# Use this script to bootstrap your build AFTER checking it out from
# source control. You should not have to use it for anything else.

### SUBMODULES ######################################################
# Make local modifications to externals checked out from other
# repositories. Skip this step if the patches have already been
# applied.
echo "Checking out submodules"
git submodule init
git submodule update
echo "Patching externals"
#patch -N -r - src/osxcart/Makefile.am src/osxcart.Makefile.am.patch

### AUTOTOOLS #######################################################
# Runs autoconf, autoheader, aclocal, automake, autopoint, libtoolize
echo "Regenerating autotools files"
autoreconf --install --force --symlink || exit 1

echo "Now run ./configure, make, and make install."
