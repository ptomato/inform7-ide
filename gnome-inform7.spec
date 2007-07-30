#
# Spec file for GNOME Inform 7
#
%define  ver     0.4
%define  rel     1.fc7
%define  prefix  /usr

Summary: An IDE for the Inform 7 interactive fiction programming language
Name: gnome-inform7
Version: %ver
Release: %rel
License: GPL
Group: Applications/Development
Source: gnome-inform7-%{ver}.tar.gz
URL: http://www.inform-fiction.org/
Packager: P.F. Chimento <philip.chimento@gmail.com>
BuildRoot: %{_tmppath}/%{name}-root

%description
GNOME Inform 7 is a port of the Mac OS X and Windows versions of the integrated
development environment for Inform 7. Inform 7 is a "natural" programming
language for writing interactive fiction (also known as text adventures.)

%prep
%setup

%build
%configure
make

%install
rm -rf %{buildroot}
%makeinstall

%clean
rm -rf %{buildroot}

%files
%defattr(-, root, root)
%doc README AUTHORS COPYING NEWS ChangeLog
%{_bindir}/gnome-inform7
%{_datadir}/applications/gnome-inform7.desktop
%{_datadir}/gnome-inform7/*
%{_datadir}/gnome-inform7/*/*
%{_datadir}/gnome-inform7/Documentation/doc_images/*
%{_datadir}/gnome-inform7/Documentation/gnome/*
%{_datadir}/gnome-inform7/Documentation/licenses/*
%{_datadir}/gnome-inform7/Documentation/Sections/*
%{_datadir}/gnome-inform7/Inform7/Extensions/Emily*/*
%{_datadir}/gnome-inform7/Inform7/Extensions/Graham*/*
%{_datadir}/gnome-inform7/Inform7/Extensions/Reserved/*
%{_datadir}/gnome-inform7/Inform7/Extensions/Reserved/*/*
%{_datadir}/gnome-inform7/Library/Natural/*
%{_datadir}/pixmaps/Inform.png
%{_datadir}/pixmaps/gnome-inform7/*

%changelog
* Sat Jun 16 2007 P.F. Chimento <philip.chimento@gmail.com>
- Repackaged for Fedora 7.
* Sat Jun 2 2007 P.F. Chimento <philip.chimento@gmail.com>
- Repackaged to release 2.
* Sun May 27 2007 P.F. Chimento <philip.chimento@gmail.com>
- Updated to version 0.3.
* Mon Apr 9 2007 P.F. Chimento <philip.chimento@gmail.com>
- Updated to version 0.2.
