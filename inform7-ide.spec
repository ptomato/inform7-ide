Name: inform7-ide
Version: 2.0.0
Release: 1%{?dist}

%define compiler_version 6M62

URL: http://inform7.com/
License: GPLv3

Source0: https://github.com/ptomato/inform7-ide/releases/download/%{version}/inform7-ide-%{version}.tar.xz
Source1: http://inform7.com/apps/%{compiler_version}/I7_%{compiler_version}_Linux_all.tar.gz

# Build requirements:
BuildRequires: meson >= 0.55
# Extra build tools
BuildRequires: gettext
BuildRequires: pkgconfig
BuildRequires: xz-lzma-compat
# Library devel packages:
BuildRequires: libplist-devel
BuildRequires: glib2-devel >= 2.52
BuildRequires: gtk3-devel >= 3.22
BuildRequires: gtksourceview4-devel
BuildRequires: gspell-devel
BuildRequires: webkit2gtk3-devel
BuildRequires: goocanvas2-devel
BuildRequires: gstreamer1-devel
BuildRequires: gstreamer1-plugins-base
BuildRequires: gstreamer1-plugins-good
BuildRequires: gstreamer1-plugins-bad-free
BuildRequires: gstreamer1-plugins-bad-free-extras

Summary: The Inform 7 interactive fiction programming environment

%description
Inform is a design system for interactive fiction based on natural
language, a new medium of writing which came out of the "text adventure"
games of the 1980s.
It has been used by many leading writers of IF over the last twenty
years, for projects ranging from historical reconstructions, through
games, to art pieces, which have won numerous awards and competitions.

# wat, definition of meson macro has builddir and srcdir swapped?!
%global _vpath_srcdir %{name}-%{version}

%define pkgdatadir %{_datadir}/%{name}
%define pkgdocdir %{_datadir}/doc/%{name}
%define pkglibdir %{_libdir}/%{name}
%define pkglibexecdir %{_libexecdir}/%{name}

%prep
%autosetup -c
%setup -T -D -a 1
cd inform7-%{compiler_version}
tar xvf inform7-compilers_%{compiler_version}_%{_arch}.tar.gz
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
rm -rf $RPM_BUILD_ROOT%{_includedir}
rm -rf $RPM_BUILD_ROOT%{_libdir}/pkgconfig
rm -rf $RPM_BUILD_ROOT%{_libexecdir}/chimara
rm -rf $RPM_BUILD_ROOT%{_datadir}/doc/chimara

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
%docdir %{pkgdocdir}
%docdir %{pkgdatadir}/Documentation
%{_datadir}/applications/com.inform7.IDE.desktop
%{_datadir}/metainfo/com.inform7.IDE.appdata.xml
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
%{_datadir}/icons/hicolor/*/actions/com.inform7.IDE.builtin.png
%{_datadir}/icons/hicolor/*/apps/com.inform7.IDE.png
%{_datadir}/icons/hicolor/*/emblems/com.inform7.IDE.materials.png
%{_datadir}/icons/hicolor/*/mimetypes/com.inform7.IDE.application-x-inform.png
%{_datadir}/icons/hicolor/*/mimetypes/com.inform7.IDE.application-x-inform-materials.png
%{_datadir}/icons/hicolor/*/mimetypes/com.inform7.IDE.application-x-inform-skein+xml.png
%{_datadir}/icons/hicolor/*/mimetypes/com.inform7.IDE.text-x-inform.png
%{_datadir}/icons/hicolor/*/mimetypes/com.inform7.IDE.text-x-natural-inform.png
%{_datadir}/icons/hicolor/*/mimetypes/com.inform7.IDE.text-x-natural-inform-extension.png
%{_datadir}/icons/hicolor/*/mimetypes/com.inform7.IDE.text-x-blurb.png
%{_datadir}/glib-2.0/schemas/com.inform7.IDE.gschema.xml
%{_datadir}/mime/packages/com.inform7.IDE.xml
%lang(de) %{_datadir}/locale/de/LC_MESSAGES/%{name}.mo
%lang(es) %{_datadir}/locale/es/LC_MESSAGES/%{name}.mo
%lang(fr) %{_datadir}/locale/fr/LC_MESSAGES/%{name}.mo
%lang(nl) %{_datadir}/locale/nl/LC_MESSAGES/%{name}.mo
%lang(nl) %{_datadir}/locale/nl/LC_MESSAGES/ratify-2.mo
%{_bindir}/inform7-ide
%{_libdir}/libchimara.so*
%{_libdir}/libratify-2.so*
%{pkglibexecdir}/cBlorb
%{pkglibdir}/frotz.so
%{pkglibdir}/git.so
%{pkglibdir}/glulxe.so
%{pkglibexecdir}/inform6
%{pkgdocdir}/inform6/*.txt
%{pkgdocdir}/inform6/ReleaseNotes.html
%{pkglibexecdir}/ni

%changelog
* Tue Apr 19 2022 Philip Chimento <philip.chimento@gmail.com> - 2.0.0-1
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
