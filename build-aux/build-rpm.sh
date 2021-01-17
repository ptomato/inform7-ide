#!/bin/sh -e

builddir=$1
if test -z "$builddir"; then
    echo "Usage: $0 BUILDDIR"
    exit 1
fi

package=$(meson introspect "$builddir" --projectinfo | jq -r .descriptive_name)
version=$(meson introspect "$builddir" --projectinfo | jq -r .version)
dist_tarball="$builddir/meson-dist/$package-$version.tar.xz"

if test ! -e "$dist_tarball"; then
    echo "Please run 'meson dist -C $builddir --no-tests' first!"
    exit 1
fi

rpmdir=$(rpm --eval %_rpmdir)
sourcedir=$(rpm --eval %_sourcedir)
srcrpmdir=$(rpm --eval %_srcrpmdir)
arch=$(rpm --eval %_target_cpu)

if test ! -e "$sourcedir/I7_${version}_Linux_all.tar.gz"; then
    wget "http://inform7.com/apps/$version/I7_${version}_Linux_all.tar.gz" \
        -P "$sourcedir"
fi

cp "$dist_tarball" "$sourcedir/"
rpmbuild -ba gnome-inform7.spec

cp "$rpmdir/$arch"/gnome-inform7-*.rpm "$srcrpmdir"/gnome-inform7-*.src.rpm .
