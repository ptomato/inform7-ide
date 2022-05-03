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
    echo "Please run 'meson dist -C $builddir --no-tests --include-subprojects' first!"
    exit 1
fi

rpmdir=$(rpm --eval %_rpmdir)
sourcedir=$(rpm --eval %_sourcedir)
srcrpmdir=$(rpm --eval %_srcrpmdir)
arch=$(rpm --eval %_target_cpu)

compiler_version=$(ls data/Inform_*_data.* | grep -oE '[0-9][A-Z][0-9]{2}')
if test ! -e "$sourcedir/I7_${compiler_version}_Linux_all.tar.gz"; then
    wget "http://inform7.com/apps/$compiler_version/I7_${compiler_version}_Linux_all.tar.gz" \
        -P "$sourcedir"
fi

cp "$dist_tarball" "$sourcedir/"
# rpath is set in meson.build so that we can run from a non-system install path;
# this is harmless otherwise, but needs to be enabled with QA_RPATHS
QA_RPATHS=1 rpmbuild -ba inform7-ide.spec

cp "$rpmdir/$arch"/inform7-ide-*.rpm "$srcrpmdir"/inform7-ide-*.src.rpm .
