#!/bin/sh -e

# Requirements: devscripts, jq, meson

builddir=$1
if test -z "$builddir"; then
    echo "Usage: $0 BUILDDIR"
    exit 1
fi

package=$(meson introspect "$builddir" --projectinfo | jq -r .descriptive_name)
version=$(meson introspect "$builddir" --projectinfo | jq -r .version)
dist_tarball="$builddir/meson-dist/$package-$version.tar.xz"
arch=$(dpkg --print-architecture)

if test ! -e "$dist_tarball"; then
    echo "Please run 'meson dist -C $builddir --no-tests --include-subprojects' first!"
    exit 1
fi

debsourcepkg="${package}_$version.orig.tar.xz"
debsourcedir="$package-$version"

mkdir -p _deb_build
cp "$dist_tarball" "_deb_build/$debsourcepkg"
cd _deb_build
tar xJf "$debsourcepkg"

cp ../src/ni/ni "$debsourcedir/src/ni/"

cd "$debsourcedir"
DEB_BUILD_MAINT_OPTIONS=hardening=-format debuild -rfakeroot -D -us -uc
release=$(head -n1 debian/changelog | cut -f2 -d\( | cut -f1 -d\) | cut -f2 -d-)
cd ..

cp "$debsourcepkg" "${package}_$version-$release.dsc" \
    "${package}_$version-$release.debian.tar.xz" \
    "${package}_$version-${release}_$arch.deb" \
    "${package}-dbgsym_$version-${release}_$arch.deb" ..
cd ..

rm -rf _deb_build
