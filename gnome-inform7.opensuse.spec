# 
# Spec file for GNOME Inform 7 on OpenSUSE. Rename to gnome-inform7.spec.
#
# Copyright (c) 2011 Malcolm J Lewis <malcolmlewis@opensuse.org>
#
# All modifications and additions to the file contributed by third parties
# remain the property of their copyright owners, unless otherwise agreed
# upon. The license for this file, and modifications and additions to the
# file, is the same license as for the pristine package itself (unless the
# license for the pristine package is not an Open Source License, in which
# case the license is the MIT License). An "Open Source License" is a
# license that conforms to the Open Source Definition (Version 1.9)
# published by the Open Source Initiative.

# Please submit bugfixes or comments via http://bugs.opensuse.org/
#

Name:           gnome-inform7
Version:        6G60
Release:        0
License:        GPL-3.0
Summary:        Inform 7 interactive fiction programming language IDE
Url:            http://inform7.com/
Group:          Development/Languages/Other
Source0:        http://voxel.dl.sourceforge.net/project/gnome-inform7/gnome-inform7/6G60/I7_6G60_GNOME_Source.tar.gz
Source1:        http://inform7.com/download/content/6G60/I7_6G60_Linux_all.tar.gz
BuildRequires:  fdupes
BuildRequires:  goocanvas-devel
BuildRequires:  graphviz
BuildRequires:  gtksourceview-devel
BuildRequires:  gtkspell-devel
BuildRequires:  intltool
BuildRequires:  libwebkitgtk-devel
BuildRequires:  lzma
BuildRequires:  pkgconfig
BuildRequires:  texlive
BuildRequires:  update-desktop-files
BuildRequires:  pkgconfig(gconf-2.0)
BuildRequires:  pkgconfig(gtk+-2.0)
BuildRequires:  pkgconfig(libxml-2.0)
Recommends:     %{name}-lang = %{version}
BuildRoot:      %{_tmppath}/%{name}-%{version}-build
%lang_package

%description
GNOME Inform 7 is a port of the Mac OS X and Windows versions of the
integrated development environment for Inform 7. Inform 7 is a "natural"
programming language for writing interactive fiction (also known as text
adventures.).

%prep
%setup -q
%setup -T -D -a 1
cd inform7-%{version}
%ifarch x86_64
tar xvf inform7-compilers_%{version}_x86_64.tar.gz
cp share/inform7/Compilers/ni ../src/ni/
%else
tar xvf inform7-compilers_%{version}_i386.tar.gz
cp share/inform7/Compilers/ni ../src/ni/
%endif
cd ..

%build
%configure --enable-manuals
make CFLAGS="$CFLAGS -fno-strict-aliasing " %{?_smp_mflags}

%install
%make_install
find %{buildroot} -type f -name "*.la" -delete -print
%suse_update_desktop_file %{buildroot}%{_datadir}/applications/gnome-inform7.desktop
%find_lang %{name}
%find_gconf_schemas
cat %{name}.schemas_list >%{name}.lst
%fdupes %{buildroot}

%pre -f %{name}.schemas_pre

%post
%desktop_database_post
%icon_theme_cache_post

%posttrans -f %{name}.schemas_posttrans

%postun
%desktop_database_postun
%icon_theme_cache_post

%preun -f %{name}.schemas_preun

%clean
%{?buildroot:rm -rf %{buildroot}}

%files -f %{name}.lst
%defattr(-,root,root)
%{_bindir}/%{name}
%{_libdir}/%{name}
%ifarch x86_64
%{_prefix}/lib/%{name}
%endif
%{_datadir}/applications/gnome-inform7.desktop
%doc %{_datadir}/doc/%{name}
%{_datadir}/%{name}
%{_datadir}/icons/hicolor/*/*

%files lang -f %{name}.lang

%changelog
* Mon Oct 10 2011 Malcolm J Lewis <malcolmlewis@opensuse.org>
- Rewrote spec file.
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
