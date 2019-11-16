#
# Spec file for GNOME Inform 7
#

Name: gnome-inform7
Version: 6M62
Release: 1.fc29

URL: http://inform7.com/
License: GPLv3

Group: Development/Languages
Source: http://inform7.com/download/content/6M62/gnome-inform7-6M62.tar.xz

#AutoReqProv: no

# Build requirements:
# Extra build tools
BuildRequires: intltool
BuildRequires: gettext
BuildRequires: pkgconfig
BuildRequires: xz-lzma-compat
# Library devel packages:
BuildRequires: libuuid-devel
BuildRequires: glib2-devel
BuildRequires: gtk2-devel
BuildRequires: gtksourceview2-devel
BuildRequires: gtkspell-devel
#BuildRequires: webkitgtk-devel
BuildRequires: goocanvas-devel
BuildRequires: gstreamer-plugins-base
BuildRequires: gstreamer1-devel
BuildRequires: gstreamer1-plugins-base
BuildRequires: gstreamer1-plugins-good
BuildRequires: gstreamer1-plugins-bad-free
BuildRequires: gstreamer1-plugins-bad-free-extras
Requires:libGL.so.1()(64bit)
Requires:libX11.so.6()(64bit)
Requires:libXcomposite.so.1()(64bit)
Requires:libXdamage.so.1()(64bit)
Requires:libXfixes.so.3()(64bit)
Requires:libXrender.so.1()(64bit)
Requires:libXt.so.6()(64bit)
Requires:libatk-1.0.so.0()(64bit)
Requires:libbz2.so.1()(64bit)
Requires:libc.so.6()(64bit)
Requires:libc.so.6(GLIBC_2.11)(64bit)
Requires:libc.so.6(GLIBC_2.14)(64bit)
Requires:libc.so.6(GLIBC_2.2.5)(64bit)
Requires:libc.so.6(GLIBC_2.3)(64bit)
Requires:libc.so.6(GLIBC_2.3.4)(64bit)
Requires:libc.so.6(GLIBC_2.4)(64bit)
Requires:libc.so.6(GLIBC_2.7)(64bit)
Requires:libcairo.so.2()(64bit)
Requires:libdl.so.2()(64bit)
Requires:libdl.so.2(GLIBC_2.2.5)(64bit)
Requires:libenchant.so.1()(64bit)
Requires:libfontconfig.so.1()(64bit)
Requires:libfreetype.so.6()(64bit)
Requires:libgailutil.so.18()(64bit)
Requires:libgcc_s.so.1()(64bit)
Requires:libgdk-x11-2.0.so.0()(64bit)
Requires:libgdk_pixbuf-2.0.so.0()(64bit)
Requires:libgio-2.0.so.0()(64bit)
Requires:libglib-2.0.so.0()(64bit)
Requires:libgmodule-2.0.so.0()(64bit)
Requires:libgobject-2.0.so.0()(64bit)
Requires:libgstapp-0.10.so.0()(64bit)
Requires:libgstaudio-0.10.so.0()(64bit)
Requires:libgstbase-0.10.so.0()(64bit)
Requires:libgstfft-0.10.so.0()(64bit)
Requires:libgstinterfaces-0.10.so.0()(64bit)
Requires:libgstpbutils-0.10.so.0()(64bit)
Requires:libgstreamer-0.10.so.0()(64bit)
Requires:libgstvideo-0.10.so.0()(64bit)
Requires:libgthread-2.0.so.0()(64bit)
Requires:libgtk-x11-2.0.so.0()(64bit)
Requires:libgtksourceview-2.0.so.0()(64bit)
Requires:libgtkspell.so.0()(64bit)
Requires:libharfbuzz.so.0()(64bit)
Requires:libicudata.so.62()(64bit)
Requires:libicui18n.so.62()(64bit)
Requires:libicuuc.so.62()(64bit)
#Requires:libjavascriptcoregtk-1.0.so.0()(64bit)
Requires:libjpeg.so.62()(64bit)
Requires:libjpeg.so.62(LIBJPEG_6.2)(64bit)
Requires:libm.so.6()(64bit)
Requires:libm.so.6(GLIBC_2.2.5)(64bit)
Requires:libm.so.6(GLIBC_2.27)(64bit)
Requires:libpango-1.0.so.0()(64bit)
Requires:libpangocairo-1.0.so.0()(64bit)
Requires:libpangoft2-1.0.so.0()(64bit)
Requires:libpng16.so.16()(64bit)
Requires:libpng16.so.16(PNG16_0)(64bit)
Requires:libpthread.so.0()(64bit)
Requires:libpthread.so.0(GLIBC_2.2.5)(64bit)
Requires:libpthread.so.0(GLIBC_2.3.2)(64bit)
Requires:libsoup-2.4.so.1()(64bit)
Requires:libsqlite3.so.0()(64bit)
Requires:libstdc++.so.6()(64bit)
Requires:libstdc++.so.6(CXXABI_1.3)(64bit)
Requires:libstdc++.so.6(CXXABI_1.3.9)(64bit)
Requires:libstdc++.so.6(GLIBCXX_3.4)(64bit)
Requires:libstdc++.so.6(GLIBCXX_3.4.15)(64bit)
Requires:libstdc++.so.6(GLIBCXX_3.4.20)(64bit)
Requires:libstdc++.so.6(GLIBCXX_3.4.21)(64bit)
Requires:libstdc++.so.6(GLIBCXX_3.4.9)(64bit)
Requires:libuuid.so.1()(64bit)
Requires:libuuid.so.1(UUID_1.0)(64bit)
#Requires:libwebkitgtk-1.0.so.0()(64bit)
Requires:libxml2.so.2()(64bit)
Requires:libxml2.so.2(LIBXML2_2.4.30)(64bit)
Requires:libxml2.so.2(LIBXML2_2.6.0)(64bit)
Requires:libxml2.so.2(LIBXML2_2.6.6)(64bit)
Requires:libxslt.so.1()(64bit)
Requires:libxslt.so.1(LIBXML2_1.0.11)(64bit)
Requires:libxslt.so.1(LIBXML2_1.0.22)(64bit)
Requires:libxslt.so.1(LIBXML2_1.0.24)(64bit)
Requires:libxslt.so.1(LIBXML2_1.1.9)(64bit)
Requires:libz.so.1()(64bit)
Requires:libz.so.1(ZLIB_1.2.0)(64bit)
Recommends: compat-libicu62
BuildRoot: %{_tmppath}/%{name}-%{version}-%{release}-root-%(%{__id_u} -n)

Summary: An IDE for the Inform 7 interactive fiction programming language

%description
GNOME Inform 7 is a port of the Mac OS X and Windows versions of the integrated
development environment for Inform 7. Inform 7 is a "natural" programming
language for writing interactive fiction (also known as text adventures.)

%prep
%setup -q

%build
%configure --enable-manuals --with-sound=gstreamer
make

%install
rm -rf %{buildroot}
make install DESTDIR=%{buildroot}
cp "%{_builddir}/%{name}-%{version}/src/webkit/lib/distribution/"*.* "%{buildroot}/%{_libdir}"

# Clean out files that should not be part of the rpm.
%{__rm} -f %{buildroot}%{_libdir}/%{name}/*.la

%post
/bin/touch --no-create %{_datadir}/icons/hicolor &>/dev/null || :
/usr/bin/update-mime-database %{_datadir}/mime &> /dev/null || :

%postun
if [ $1 -eq 0 ] ; then
    /bin/touch --no-create %{_datadir}/icons/hicolor &>/dev/null
    /usr/bin/gtk-update-icon-cache %{_datadir}/icons/hicolor &>/dev/null || :
    /usr/bin/glib-compile-schemas %{_datadir}/glib-2.0/schemas &> /dev/null || :
fi
/usr/bin/update-mime-database %{_datadir}/mime &> /dev/null || :

%posttrans
/usr/bin/gtk-update-icon-cache %{_datadir}/icons/hicolor &>/dev/null || :
/usr/bin/glib-compile-schemas %{_datadir}/glib-2.0/schemas &> /dev/null || :

%clean
rm -rf %{buildroot}

%files
%defattr(-, root, root)
%define pkgdatadir %{_datadir}/%{name}
%define pkgdocdir %{_datadir}/doc/%{name}
%define pkglibdir %{_libdir}/%{name}
%define pkglibexecdir %{_libexecdir}/%{name}
%docdir %{pkgdocdir}
%docdir %{pkgdatadir}/Documentation
%{_datadir}/applications/com.inform7.IDE.desktop
%{_datadir}/metainfo/com.inform7.IDE.appdata.xml
%{pkgdocdir}/AUTHORS.md
%{pkgdocdir}/ChangeLog
%{pkgdocdir}/COPYING
%{pkgdocdir}/README.md
%{pkgdatadir}/uninstall_manifest.txt
%{pkgdatadir}/Documentation/*.html
%{pkgdatadir}/Documentation/*.css
%{pkgdatadir}/Documentation/manifest.txt
%{pkgdatadir}/Resources/bg_images/*.png
%{pkgdatadir}/Resources/bg_images/*.gif
%{pkgdatadir}/Resources/doc_images/*.png
%{pkgdatadir}/Resources/doc_images/*.jpg
%{pkgdatadir}/Resources/doc_images/*.tif
%{pkgdatadir}/Resources/map_icons/*.png
%{pkgdatadir}/Resources/outcome_images/*.png
%{pkgdatadir}/Resources/scene_icons/*.png
%{pkgdatadir}/Resources/Welcome*Background.png
%{pkgdatadir}/Resources/en/*.html
%{pkgdatadir}/Extensions/Emily*Short/*.i7x
%{pkgdatadir}/Extensions/Eric*Eve/Epistemology.i7x
%{pkgdatadir}/Extensions/Graham*Nelson/*.i7x
%{pkgdatadir}/I6T/*.i6t
%{pkgdatadir}/Languages/*/Syntax.preform
%{pkgdatadir}/Languages/*/about.txt
%{pkgdatadir}/Languages/*/flag.png
%{pkgdatadir}/Miscellany/*.html
%{pkgdatadir}/Miscellany/*.jpg
%{pkgdatadir}/Miscellany/*.pdf
%{pkgdatadir}/Templates/*/*.html
%{pkgdatadir}/Templates/*/*.css
%{pkgdatadir}/Templates/*/*.js
%{pkgdatadir}/Templates/*/(manifest).txt
%{pkgdatadir}/Templates/Quixe/waiting.gif
%{pkgdatadir}/Templates/Vorple/soundmanager2.swf
%{pkgdatadir}/highlighting/*.lang
%{pkgdatadir}/Documentation/licenses/*.html
%{pkgdatadir}/styles/*.xml
%{pkgdatadir}/ui/*.ui
%{pkgdatadir}/ui/*.xml
%{_datadir}/icons/hicolor/*/actions/inform7-builtin.png
%{_datadir}/icons/hicolor/*/apps/inform7.png
%{_datadir}/icons/hicolor/*/emblems/inform7-materials.png
%{_datadir}/icons/hicolor/*/mimetypes/application-x-inform.png
%{_datadir}/icons/hicolor/*/mimetypes/application-x-inform-materials.png
%{_datadir}/icons/hicolor/*/mimetypes/application-x-inform-skein+xml.png
%{_datadir}/icons/hicolor/*/mimetypes/text-x-inform.png
%{_datadir}/icons/hicolor/*/mimetypes/text-x-natural-inform.png
%{_datadir}/icons/hicolor/*/mimetypes/text-x-natural-inform-extension.png
%{_datadir}/icons/hicolor/*/mimetypes/text-x-blurb.png
%{_datadir}/glib-2.0/schemas/com.inform7.IDE.gschema.xml
%{_datadir}/mime/packages/inform7.xml
%lang(de) %{_datadir}/locale/de/LC_MESSAGES/%{name}.mo
%lang(es) %{_datadir}/locale/es/LC_MESSAGES/%{name}.mo
%lang(fr) %{_datadir}/locale/fr/LC_MESSAGES/%{name}.mo
%lang(nl) %{_datadir}/locale/nl/LC_MESSAGES/%{name}.mo
%{_bindir}/gnome-inform7
%{pkglibexecdir}/cBlorb
%{pkgdocdir}/cBlorb/Complete.html
%{pkgdocdir}/cBlorb/crumbs.gif
%{pkgdocdir}/cBlorb/inweb.css
%{pkglibdir}/frotz.so
%{pkgdocdir}/frotz/AUTHORS
%{pkgdocdir}/frotz/COPYING
%{pkgdocdir}/frotz/README
%{pkgdocdir}/frotz/TODO
%{pkglibdir}/git.so
%{pkgdocdir}/git/README.txt
%{pkglibdir}/glulxe.so
%{pkgdocdir}/glulxe/README
%{pkglibexecdir}/inform6
%{pkgdocdir}/inform6/*.txt
%{pkgdocdir}/inform6/ReleaseNotes.html
%{pkglibexecdir}/ni
%{_libdir}/libwebkitgtk-1.0.so.0
%{_libdir}/libwebkitgtk-1.0.so.0.17.5
%{_libdir}/libwebkitgtk-1.0.so
%{_libdir}/libjavascriptcoregtk-1.0.so
%{_libdir}/libjavascriptcoregtk-1.0.so.0
%{_libdir}/libjavascriptcoregtk-1.0.so.0.13.11

%changelog
* Sun Jan 10 2016 Philip Chimento <philip.chimento@gmail.com> - 6M62-1
- Repackaged to Build 6M62.
* Sat Oct 10 2015 Philip Chimento <philip.chimento@gmail.com>
- Updated documentation files.
* Sun Sep 7 2014 Philip Chimento <philip.chimento@gmail.com> - 6L38-1
- Repackaged to Build 6L38.
* Wed May 7 2014 Philip Chimento <philip.chimento@gmail.com> - 6L02-1
- Repackaged to Build 6L02.
* Sun Feb 12 2012 P. F. Chimento <philip.chimento@gmail.com>
- Changed 'lzma' requirement to 'xz-lzma-compat'.
* Wed Jan 12 2011 P.F. Chimento <philip.chimento@gmail.com>
- Updated build requirements.
- Updated packing list.
* Fri Dec 17 2010 P.F. Chimento <philip.chimento@gmail.com>
- Changed files after merge of development branch.
- Added scriptlet for icon cache updating.
- Updated scriptlet for GConf schemas.
* Tue Oct 26 2010 P.F. Chimento <philip.chimento@gmail.com>
- Added Quixe and Eric Eve directories to packing list.
* Mon Oct 4 2010 P.F. Chimento <philip.chimento@gmail.com>
- Use gconf RPM macros instead of shell scripts.
- List build requirements.
* Sat Jul 3 2010 P.F. Chimento <philip.chimento@gmail.com>
- Fixed rpmlint warnings.
* Thu Jun 24 2010 P.F. Chimento <philip.chimento@gmail.com>
- Added Parchment directory to packing list.
* Fri Apr 10 2009 P.F. Chimento <philip.chimento@gmail.com>
- Overhauled build process.
* Mon Feb 23 2009 P.F. Chimento <philip.chimento@gmail.com>
- Added the gtkterp-git binary to the packing list.
* Sat Dec 6 2008 P.F. Chimento <philip.chimento@gmail.com>
- Repackaged to release .1 of Public Beta Build 5U92.
* Sun Sep 14 2008 P.F. Chimento <philip.chimento@gmail.com>
- Added scriptlets for GConf2 schemas processing.
* Fri Sep 12 2008 P.F. Chimento <philip.chimento@gmail.com>
- Updated to Public Beta Build 5U92.
* Sat May 3 2008 P.F. Chimento <philip.chimento@gmail.com>
- Fedora 8 release bumped to 2, replacing outdated Glulx Entry Points.
* Wed Apr 30 2008 P.F. Chimento <philip.chimento@gmail.com>
- Updated to Public Beta Build 5T18.
* Mon Dec 3 2007 P.F. Chimento <philip.chimento@gmail.com>
- Updated to Public Beta Build 5J39.
* Tue Nov 13 2007 P.F. Chimento <philip.chimento@gmail.com>
- Updated to Public Beta Build 5G67.
* Sat Aug 18 2007 P.F. Chimento <philip.chimento@gmail.com>
- Updated to version 0.4.
* Sat Jun 16 2007 P.F. Chimento <philip.chimento@gmail.com>
- Repackaged for Fedora 7.
* Sat Jun 2 2007 P.F. Chimento <philip.chimento@gmail.com>
- Repackaged to release 2.
* Sun May 27 2007 P.F. Chimento <philip.chimento@gmail.com>
- Updated to version 0.3.
* Mon Apr 9 2007 P.F. Chimento <philip.chimento@gmail.com>
- Updated to version 0.2.
