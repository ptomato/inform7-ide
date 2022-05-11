#!/bin/sh -e

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

if test ! -e "$dist_tarball"; then
    echo "Please run 'meson dist -C $builddir --no-tests --include-subprojects' first!"
    exit 1
fi

rpmdir=$(rpm --eval %_rpmdir)
sourcedir=$(rpm --eval %_sourcedir)
srcrpmdir=$(rpm --eval %_srcrpmdir)
arch=$(rpm --eval %_target_cpu)

if test ! -e "$sourcedir/$inweb_commit.zip"; then
    wget "https://github.com/ganelson/inweb/archive/$inweb_commit.zip" \
        -P "$sourcedir"
fi
if test ! -e "$sourcedir/$intest_commit.zip"; then
    wget "https://github.com/ganelson/intest/archive/$intest_commit.zip" \
        -P "$sourcedir"
fi
if test ! -e "$sourcedir/$inform_commit.zip"; then
    wget "https://github.com/ganelson/inform/archive/$inform_commit.zip" \
        -P "$sourcedir"
fi

cp "$dist_tarball" "$sourcedir/"
# rpath is set in meson.build so that we can run from a non-system install path;
# this is harmless otherwise, but needs to be enabled with QA_RPATHS
QA_RPATHS=1 rpmbuild -ba inform7-ide.spec

cp "$rpmdir/$arch"/inform7-ide-*.rpm "$srcrpmdir"/inform7-ide-*.src.rpm .
