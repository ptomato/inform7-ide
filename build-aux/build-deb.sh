#!/bin/sh -e

# Requirements: devscripts, jq, meson

builddir=$1
if test -z "$builddir"; then
    echo "Usage: $0 BUILDDIR"
    exit 1
fi

inweb_commit=2aca05e8e28c6385ade2a0637d9a79adbede9bb5
intest_commit=06c8a4f57f104fa12abcf478371748b5bd829c7b
inform_commit=7f7deec532d7c92ce14c2ba161f1fa6ae0677a85

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

cd "$debsourcedir"

# Debian has strict requirements for how subsequent source packages beyond the
# first can be packed and unpacked. The tarballs from GitHub's API endpoint are
# not sufficient. We have to unpack and repack them without the leading path
# component, or dpkg-unpack will not unpack them correctly.

if test ! -e "../$package_$version.orig-inweb.tar.gz"; then
    wget "https://api.github.com/repos/ganelson/inweb/tarball/$inweb_commit" \
        -O inweb.tar.gz
    mkdir inweb
    tar xzf inweb.tar.gz -C inweb --strip-components=1
    tar czf "../$package_$version.orig-inweb.tar.gz" inweb
    rm inweb.tar.gz
fi
if test ! -e "../$package_$version.orig-intest.tar.gz"; then
    wget "https://api.github.com/repos/ganelson/intest/tarball/$intest_commit" \
        -O intest.tar.gz
    mkdir intest
    tar xzf intest.tar.gz -C intest --strip-components=1
    tar czf "../$package_$version.orig-intest.tar.gz" intest
    rm intest.tar.gz
fi
if test ! -e "../$package_$version.orig-inform.tar.gz"; then
    wget "https://api.github.com/repos/ganelson/inform/tarball/$inform_commit" \
        -O inform.tar.gz
    mkdir inform
    tar xzf inform.tar.gz -C inform --strip-components=1
    tar czf "../$package_$version.orig-inform.tar.gz" inform
    rm inform.tar.gz
fi

DEB_BUILD_MAINT_OPTIONS=hardening=-format debuild -rfakeroot -D -us -uc
release=$(head -n1 debian/changelog | cut -f2 -d\( | cut -f1 -d\) | cut -f2 -d-)
cd ..

ls -la
cp "$debsourcepkg" "$package_$version.orig-inweb.tar.gz" \
    "$package_$version.orig-intest.tar.gz" \
    "$package_$version.orig-inform.tar.gz" "${package}_$version-$release.dsc" \
    "${package}_$version-$release.debian.tar.xz" \
    "${package}_$version-${release}_$arch.deb" \
    "${package}-dbgsym_$version-${release}_$arch.ddeb" ..
cd ..

rm -rf _deb_build
