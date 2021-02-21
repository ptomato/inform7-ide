project('gnome-inform7', 'c', version: '6M62', license: 'GPL-3.0-or-later',
    meson_version: '>=0.55.0')

appname = meson.project_name()
version = meson.project_version()

gnome = import('gnome')
i18n = import('i18n')

cc = meson.get_compiler('c')

### CHECK FOR PROPER NI EXECUTABLE #############################################

# not "required" because we want to print a more informative error message
ni = find_program('src/ni/ni', required: false)
if not ni.found()
    error('''ni not found in src/ni.
    IMPORTANT NOTE! You must download version @0@ of the appropriate
    compiler package from inform7.org and put the file `ni' into the
    directory `@1@/src/ni'.
    (Try http://inform7.com/apps/@0@/I7_@0@_Linux_all.tar.gz)
    '''.format(version, meson.current_source_dir()))
endif
ni_output = run_command(ni).stdout()
ni_version = ni_output.split()[3]
if ni_version != version
    error('''You have version @0@ of ni
    in src/ni. This version of Inform 7 will only work with version @1@.
    IMPORTANT NOTE! You must download version @1@ of the appropriate
    compiler package from inform7.com and put the file `ni' into the
    directory `@2@/src/ni'.
    (Try http://inform7.com/apps/@1@/I7_@1@_Linux_all.tar.gz)
    '''.format(ni_version, version, meson.current_source_dir()))
endif

### DEFINE VARIABLES ###########################################################

inform_tarball = files('data/Inform_@0@_data.tar.lz'.format(version))

datadir = get_option('datadir')
pkgdatadir = datadir / appname
pkglibdir = get_option('libdir') / appname
pkglibexecdir = get_option('libexecdir') / appname
pkgdocsdir = datadir / 'doc' / appname

prefix = get_option('prefix')
abs_datadir = prefix / datadir
abs_pkglibexecdir = prefix / pkglibexecdir
abs_localedir = prefix / datadir / 'locale'
abs_plugindir = prefix / get_option('libdir') / appname

add_project_arguments('-DLOCALEDIR="@0@"'.format(abs_localedir),
    '-DPACKAGE_LOCALE_DIR="@0@"'.format(abs_localedir),
    '-DPACKAGE_DATA_DIR="@0@"'.format(abs_datadir),
    '-DPACKAGE_LIBEXEC_DIR="@0@"'.format(abs_pkglibexecdir),
    language: 'c')

top_include = include_directories('.')

### CHECK FOR DEPENDENCIES #####################################################

date = find_program('date')
find_program('tar')
find_program('xz')

libm = cc.find_library('m', required: false)

libxml = dependency('libxml-2.0')
glib = dependency('glib-2.0', version: '>=2.52')
gio = dependency('gio-2.0', version: '>=2.30')
gdk = dependency('gdk-3.0')
gdkpixbuf = dependency('gdk-pixbuf-2.0', version: '>=2.6')
pango = dependency('pango')
gtk = dependency('gtk+-3.0', version: '>=3.22')
gtksourceview = dependency('gtksourceview-3.0')
gtkspell = dependency('gtkspell3-3.0')
goocanvas = dependency('goocanvas-2.0')
webkit = dependency('webkit2gtk-4.0')
plist = dependency('libplist-2.0', required: false)
if not plist.found()
    plist = dependency('libplist')
endif

libchimara = dependency('chimara', fallback: ['chimara', 'libchimara_dep'],
    default_options: ['bocfel=false', 'nitfol=false', 'player=false',
        'introspection=false', 'plugindir=@0@'.format(pkglibdir)])

### CONFIGURE ##################################################################

config = configuration_data()

config.set_quoted('PACKAGE', appname)
config.set_quoted('PACKAGE_NAME', appname)
config.set_quoted('GETTEXT_PACKAGE', appname)
config.set_quoted('PACKAGE_VERSION', version)

# Inform 6 version that goes along with this release
config.set_quoted('INFORM6_COMPILER_VERSION', '6.33')

# Get current year for updating copyrights
copyright_year = run_command(date, '+%Y').stdout().strip()
config.set_quoted('COPYRIGHT_YEAR', copyright_year)

if glib.version().version_compare('>=2.60')
    config.set('GLIB_VERSION_MIN_REQUIRED', 'GLIB_VERSION_2_60')
endif

configure_file(output: 'config.h', configuration: config)

### BUILD AND INSTALL ##########################################################

local_schemas = gnome.compile_schemas()  # Needed for tests

subdir('src')
subdir('po')

i18n.merge_file(input: 'com.inform7.IDE.desktop.in',
    output: 'com.inform7.IDE.desktop', type: 'desktop', po_dir: 'po',
    install: true, install_dir: datadir / 'applications')
i18n.merge_file(input: 'data/mime/inform7.xml.in', output: 'inform7.xml',
    type: 'xml', po_dir: 'po',
    install: true, install_dir: datadir / 'mime' / 'packages')

install_data('com.inform7.IDE.appdata.xml', install_dir: datadir / 'metainfo')

install_data('com.inform7.IDE.gschema.xml',
    install_dir: datadir / 'glib-2.0' / 'schemas')

install_data('AUTHORS.md', 'ChangeLog', 'COPYING', 'README.md',
    install_dir: pkgdocsdir)

install_subdir('data/highlighting', install_dir: pkgdatadir)
install_subdir('data/styles', install_dir: pkgdatadir,
    exclude_files: ['styles.its', 'styles.pot', 'update-pot.sh'])

# Named Icons
# -----------
# com.inform7.IDE.png: named application icon (inform.iconset)
# inform7-builtin.png: named icon for built-in extension
# inform7-materials.png: named emblem for materials folder
#
# MIME types
# ----------
# application/x-inform: Inform 7 project (informfile.iconset)
# application/x-inform-materials: Materials folder (materialsfile.iconset)
# application/x-inform-skein+xml: Skein file (skeinfile.iconset)
# text/x-inform: Inform 6 source code (i6file.iconset, inffile.iconset)
# text/x-natural-inform: Inform 7 source code (i7file.iconset, nifile.iconset)
# text/x-natural-inform-extension: Extension file (i7xfile.iconset)
# text/x-blurb: Blurb file (blurbfile.iconset)

icondir = datadir / 'icons' / 'hicolor'
install_subdir('pixmaps/16x16', install_dir: icondir)
install_subdir('pixmaps/32x32', install_dir: icondir)
install_subdir('pixmaps/48x48', install_dir: icondir)
install_subdir('pixmaps/64x64', install_dir: icondir)
install_subdir('pixmaps/128x128', install_dir: icondir)
install_subdir('pixmaps/256x256', install_dir: icondir)

meson.add_install_script('build-aux/unpack-inform-data.sh', inform_tarball,
    pkgdatadir)
# Script adapted from GNOME Notes's build system
meson.add_install_script('build-aux/meson_post_install.py', abs_datadir)
