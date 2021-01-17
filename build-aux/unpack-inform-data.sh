#!/bin/bash -e

inform_tarball=$1
pkgdatadir="$MESON_INSTALL_DESTDIR_PREFIX/$2"
mkdir -p "$pkgdatadir"
cd "$pkgdatadir"
xz -F lzma -dc $inform_tarball \
    | tar --exclude=Documentation* --exclude=Resources* \
        --no-same-owner --mode=a=rX,u+w -x -v \
    > uninstall_manifest.txt

# Append the list of files to Meson's install log so that Meson will
# automatically uninstall them when requested
cat uninstall_manifest.txt \
    | sed -e "s|^|$MESON_INSTALL_PREFIX/$2/|" \
    >> "$MESON_BUILD_ROOT/meson-logs/install-log.txt"
