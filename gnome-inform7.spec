#
# Spec file for GNOME Inform 7
#

Name: gnome-inform7
Version: 6M62
Release: 2%{?dist}

URL: http://inform7.com/
License: GPLv3

Source0: https://github.com/ptomato/gnome-inform7/releases/download/%{version}/gnome-inform7-%{version}.tar.xz
Source1: http://inform7.com/apps/%{version}/I7_%{version}_Linux_all.tar.gz

# Build requirements:
BuildRequires: meson >= 0.55
# Extra build tools
BuildRequires: gettext
BuildRequires: pkgconfig
BuildRequires: xz-lzma-compat
# Library devel packages:
BuildRequires: libuuid-devel
BuildRequires: glib2-devel >= 2.44
BuildRequires: gtk3-devel >= 3.22
BuildRequires: gtksourceview3-devel
BuildRequires: gtkspell3-devel
BuildRequires: webkit2gtk3-devel
BuildRequires: goocanvas2-devel
BuildRequires: gstreamer1-devel
BuildRequires: gstreamer1-plugins-base
BuildRequires: gstreamer1-plugins-good
BuildRequires: gstreamer1-plugins-bad-free
BuildRequires: gstreamer1-plugins-bad-free-extras

Summary: An IDE for the Inform 7 interactive fiction programming language

%description
GNOME Inform 7 is a port of the Mac OS X and Windows versions of the integrated
development environment for Inform 7. Inform 7 is a "natural" programming
language for writing interactive fiction (also known as text adventures.)

# wat, definition of meson macro has builddir and srcdir swapped?!
%global _vpath_srcdir %{name}-%{version}

%prep
%autosetup -c
%setup -T -D -a 1
cd inform7-%{version}
tar xvf inform7-compilers_%{version}_%{_arch}.tar.gz
cp share/inform7/Compilers/ni ../%{_vpath_srcdir}/src/ni/
cd ..

%build
%set_build_flags
%{shrink:%{__meson} --buildtype=plain --prefix=%{_prefix} --libdir=%{_libdir}
   --libexecdir=%{_libexecdir} --bindir=%{_bindir} --sbindir=%{_sbindir}
   --includedir=%{_includedir} --datadir=%{_datadir} --mandir=%{_mandir}
   --infodir=%{_infodir} --localedir=%{_datadir}/locale
   --sysconfdir=%{_sysconfdir} --localstatedir=%{_localstatedir}
   --sharedstatedir=%{_sharedstatedir} --wrap-mode=%{__meson_wrap_mode}
   --auto-features=%{__meson_auto_features} %{_vpath_builddir} %{_vpath_srcdir}}
%meson_build

%install
%meson_install

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
%{pkgdatadir}/styles/*.xml
%{_datadir}/icons/hicolor/*/actions/inform7-builtin.png
%{_datadir}/icons/hicolor/*/apps/com.inform7.IDE.png
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

%changelog
* Sun Jan 17 2021 Philip Chimento <philip.chimento@gmail.com> - 6M62-2
- Bring spec file up to date with newer library versions and build system
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
