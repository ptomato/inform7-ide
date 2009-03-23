#
# Spec file for GNOME Inform 7
#
%define  ver     5U92.1
%define  rel     1.fc10
%define  prefix  /usr

Summary: An IDE for the Inform 7 interactive fiction programming language
Name: gnome-inform7
Version: %ver
Release: %rel
License: GPL
Group: Applications/Development
Source: gnome-inform7-%{ver}.tar.gz
URL: http://www.inform-fiction.org/
Packager: P. F. Chimento <philip.chimento@gmail.com>
BuildRoot: %{_tmppath}/%{name}-%{version}-%{release}-root-%(%{__id_u} -n)
Requires(pre): GConf2
Requires(post): GConf2
Requires(preun): GConf2

%description
GNOME Inform 7 is a port of the Mac OS X and Windows versions of the integrated
development environment for Inform 7. Inform 7 is a "natural" programming
language for writing interactive fiction (also known as text adventures.)

%prep
%setup -q

%build
%configure
make

%pre
if [ "$1" -gt 1 ] ; then
  GCONF_CONFIG_SOURCE=`gconftool-2 --get-default-source` \
    gconftool-2 --makefile-uninstall-rule \
    %{_sysconfdir}/gconf/schemas/[NAME] .schemas >/dev/null || :
fi

%install
rm -rf %{buildroot}
export GCONF_DISABLE_MAKEFILE_SCHEMA_INSTALL=1
%makeinstall
unset GCONF_DISABLE_MAKEFILE_SCHEMA_INSTALL

%post
GCONF_CONFIG_SOURCE=`gconftool-2 --get-default-source` \
  gconftool-2 --makefile-install-rule \
  %{_sysconfdir}/gconf/schemas/%{name}.schemas &>/dev/null || :

%preun
if [ "$1" -eq 0 ] ; then
  GCONF_CONFIG_SOURCE=`gconftool-2 --get-default-source` \
    gconftool-2 --makefile-uninstall-rule \
    %{_sysconfdir}/gconf/schemas/%{name}.schemas &>/dev/null || :
fi

%clean
rm -rf %{buildroot}

%files
%defattr(-, root, root)
%doc README AUTHORS COPYING NEWS THANKS ChangeLog
%{_bindir}/gnome-inform7
%{_bindir}/gtkterp-frotz
%{_bindir}/gtkterp-glulxe
%{_bindir}/gtkterp-git
%{_docdir}/gtkterp/*
%{_datadir}/applications/gnome-inform7.desktop
%{_datadir}/gnome-inform7/*.png
%{_datadir}/gnome-inform7/*.lang
%{_datadir}/gnome-inform7/styles/*.xml
%{_datadir}/gnome-inform7/Compilers/cBlorb
%{_datadir}/gnome-inform7/Compilers/inform-6.31-biplatform
%{_datadir}/gnome-inform7/Compilers/ni
%{_datadir}/gnome-inform7/Documentation/*.html
%{_datadir}/gnome-inform7/Documentation/manifest.txt
%{_datadir}/gnome-inform7/Documentation/doc_images/*
%{_datadir}/gnome-inform7/Documentation/gnome/gnome.html
%{_datadir}/gnome-inform7/Documentation/licenses/*.html
%{_datadir}/gnome-inform7/Documentation/Sections/*.html
%{_datadir}/gnome-inform7/Inform7/Extensions/David*/*
%{_datadir}/gnome-inform7/Inform7/Extensions/Emily*/*
%{_datadir}/gnome-inform7/Inform7/Extensions/Graham*/*
%{_datadir}/gnome-inform7/Inform7/Extensions/Reserved/*.i6t
%{_datadir}/gnome-inform7/Inform7/Extensions/Reserved/*.html
%{_datadir}/gnome-inform7/Inform7/Extensions/Reserved/IntroductionToIF.pdf
%{_datadir}/gnome-inform7/Inform7/Extensions/Reserved/Templates/Standard.html
%{_datadir}/gnome-inform7/Inform7/Extensions/Reserved/Templates/Standard-Source.html
%{_datadir}/gnome-inform7/Library/Natural/*.h
%{_datadir}/gnome-inform7/map_icons/*.png
%{_datadir}/gnome-inform7/scene_icons/*.png
%{_datadir}/pixmaps/Inform.png
%{_datadir}/pixmaps/gnome-inform7/*
%{_sysconfdir}/gconf/schemas/*
%{_datadir}/locale/es/LC_MESSAGES/gnome-inform7.mo

%changelog
* Mon Feb 23 2008 P.F. Chimento <philip.chimento@gmail.com>
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
