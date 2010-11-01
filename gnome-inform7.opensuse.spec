# 
# Spec file for GNOME Inform 7 on OpenSUSE. Rename to gnome-inform7.spec.
#

Name: gnome-inform7
Version: 6F95
Release: 1

URL: http://inform7.com/
License: GPLv3

Group: Development/Languages
Source: I7_%{version}_GNOME_Source.tar.gz
# Packager: P. F. Chimento <philip.chimento@gmail.com>

# Build requirements
# For building manuals
BuildRequires: texlive, texlive-bin
BuildRequires: graphviz
# Extra build tools
BuildRequires: intltool
BuildRequires: pkg-config
BuildRequires: lzma
# Library devel packages
BuildRequires: libuuid-devel
BuildRequires: glib2-devel
BuildRequires: gtk2-devel
BuildRequires: gtksourceview-devel
BuildRequires: gtkspell-devel
BuildRequires: gtkhtml2-devel
BuildRequires: dbus-1-glib-devel
%if 0%{?suse_version} >= 1120
BuildRequires: libSDL_sound-devel, libSDL_mixer-devel
%else
BuildRequires: SDL_sound-devel, SDL_mixer-devel
%endif
BuildRoot: %{_tmppath}/%{name}-%{version}-build

%gconf_schemas_prereq

Summary: An IDE for the Inform 7 interactive fiction programming language

%description
GNOME Inform 7 is a port of the Mac OS X and Windows versions of the integrated
development environment for Inform 7. Inform 7 is a "natural" programming
language for writing interactive fiction (also known as text adventures.)

%prep
%setup -q

%build
%configure --enable-manuals
make %{?_smp_mflags}

%pre -f %{name}.schemas_pre

%install
%makeinstall
%find_gconf_schemas

%preun -f %{name}.schemas_preun

%posttrans -f %{name}.schemas_posttrans

%clean
rm -rf %{buildroot}

%files -f %{name}.schemas_list

%files
%defattr(-,root,root,-)
%define pkgdatadir %{_datadir}/%{name}
%define pkgdocdir %{_datadir}/doc/%{name}
%define pkglibexecdir %{_libexecdir}/%{name}
%docdir %{pkgdocdir}
%docdir %{pkgdatadir}/Documentation
%{_datadir}/applications/%{name}.desktop
%{pkgdocdir}/AUTHORS 
%{pkgdocdir}/ChangeLog 
%{pkgdocdir}/COPYING 
%{pkgdocdir}/NEWS 
%{pkgdocdir}/README 
%{pkgdocdir}/THANKS 
%{pkgdocdir}/TODO
%{pkgdatadir}/uninstall_manifest.txt
%{pkgdatadir}/Documentation/*.html
%{pkgdatadir}/Documentation/*.png
%{pkgdatadir}/Documentation/*.gif
%{pkgdatadir}/Documentation/manifest.txt
%{pkgdatadir}/Documentation/doc_images/*.png
%{pkgdatadir}/Documentation/doc_images/*.jpg
%{pkgdatadir}/Documentation/doc_images/*.tif
%{pkgdatadir}/Documentation/map_icons/*.png
%{pkgdatadir}/Documentation/scene_icons/*.png
%{pkgdatadir}/Documentation/Sections/*.html
%{pkgdatadir}/Extensions/David*Fisher/English.i7x
%{pkgdatadir}/Extensions/Emily*Short/*.i7x
%{pkgdatadir}/Extensions/Eric*Eve/Epistemology.i7x
%{pkgdatadir}/Extensions/Graham*Nelson/*.i7x
%{pkgdatadir}/Extensions/Reserved/*.i6t
%{pkgdatadir}/Extensions/Reserved/*.jpg
%{pkgdatadir}/Extensions/Reserved/*.html
%{pkgdatadir}/Extensions/Reserved/IntroductionToIF.pdf
%{pkgdatadir}/Extensions/Reserved/Templates/Classic/*.html
%{pkgdatadir}/Extensions/Reserved/Templates/Standard/*.html
%{pkgdatadir}/Extensions/Reserved/Templates/Standard/style.css
%{pkgdatadir}/Extensions/Reserved/Templates/Parchment/*.js
%{pkgdatadir}/Extensions/Reserved/Templates/Parchment/parchment.css
%{pkgdatadir}/Extensions/Reserved/Templates/Parchment/(manifest).txt
%{pkgdatadir}/Extensions/Reserved/Templates/Quixe/*.js
%{pkgdatadir}/Extensions/Reserved/Templates/Quixe/*.css
%{pkgdatadir}/Extensions/Reserved/Templates/Quixe/waiting.gif
%{pkgdatadir}/Extensions/Reserved/Templates/Quixe/(manifest).txt
%{pkgdatadir}/languages/*.lang
%{pkgdatadir}/Documentation/licenses/*.html
%{pkgdatadir}/styles/*.xml
%{_datadir}/pixmaps/Inform.png
%{_datadir}/pixmaps/%{name}/*.png
%lang(es) %{_datadir}/locale/es/LC_MESSAGES/%{name}.mo
%{_bindir}/gnome-inform7
%{pkglibexecdir}/cBlorb
%{pkgdocdir}/cBlorb/Complete.pdf
%{pkglibexecdir}/gtkterp-frotz
%{pkgdocdir}/frotz/AUTHORS
%{pkgdocdir}/frotz/COPYING
%{pkgdocdir}/frotz/README
%{pkgdocdir}/frotz/TODO
%config(noreplace) %{_sysconfdir}/gtkterp.ini
%{pkgdocdir}/garglk/*.txt
%{pkgdocdir}/garglk/TODO
%{pkglibexecdir}/gtkterp-git
%{pkgdocdir}/git/README.txt
%{pkglibexecdir}/gtkterp-glulxe
%{pkgdocdir}/glulxe/README
%{pkglibexecdir}/inform-6.31-biplatform
%{pkgdocdir}/inform6/readme.txt
%{pkglibexecdir}/ni

%changelog
* Mon Nov 1 2010 P.F. Chimento <philip.chimento@gmail.com>
- Updated OpenSUSE version of spec file.
* Tue Oct 26 2010 P.F. Chimento <philip.chimento@gmail.com>
- Added Quixe and Eric Eve directories to packing list.
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
* Sun Sep 12 2008 P.F. Chimento <philip.chimento@gmail.com>
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
