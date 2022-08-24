#!/bin/sh -e

builddir=$1
if test -z "$builddir"; then
    echo "Usage: $0 BUILDDIR"
    exit 1
fi

inweb_ref=v7.2.0
intest_ref=v2.1.0
inform_ref=v10.1.2

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

if ! ls "$sourcedir"/ganelson-inweb-$inweb_ref-*.tar.gz >/dev/null; then
    wget "https://api.github.com/repos/ganelson/inweb/tarball/$inweb_ref" \
        --content-disposition -P "$sourcedir"
fi
if ! ls "$sourcedir"/ganelson-intest-$intest_ref-*.tar.gz >/dev/null; then
    wget "https://api.github.com/repos/ganelson/intest/tarball/$intest_ref" \
        --content-disposition -P "$sourcedir"
fi
if ! ls "$sourcedir"/ganelson-inform-$inform_ref-*.tar.gz >/dev/null; then
    wget "https://api.github.com/repos/ganelson/inform/tarball/$inform_ref" \
        --content-disposition -P "$sourcedir"
fi

cp "$dist_tarball" "$sourcedir/"
# rpath is set in meson.build so that we can run from a non-system install path;
# this is harmless otherwise, but needs to be enabled with QA_RPATHS
QA_RPATHS=1 rpmbuild -ba inform7-ide.spec

cp "$rpmdir/$arch"/inform7-ide-*.rpm "$srcrpmdir"/inform7-ide-*.src.rpm .
