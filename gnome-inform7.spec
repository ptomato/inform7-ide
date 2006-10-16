#
# Spec file for GNOME Inform 7
#
%define  ver     0.1
%define  rel     1
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
Requires: wine frotz

%description
GNOME Inform 7 is a port of the Mac OS X and Windows versions of the integrated
development environment for Inform 7. Inform 7 is a "natural" programming
language for writing interactive fiction (also known as text adventures.)

%prep
%setup

%build
%configure
#pushd src/inform
#%configure
#popd
make

%install
rm -rf %{buildroot}
%makeinstall

%clean
rm -rf %{buildroot}

%files
%defattr(-, root, root)
%doc README AUTHORS COPYING NEWS ChangeLog
%{_bindir}/*
%{_datadir}/gnome-inform7/*
%{_datadir}/gnome-inform7/*/*
%{_datadir}/gnome-inform7/*/*/*
%{_datadir}/gnome-inform7/*/*/*/*
%{_datadir}/inform/*/*/*
%{_datadir}/pixmaps/*
%{_datadir}/pixmaps/*/*
%{_includedir}/*
%{_datadir}/applications/*
