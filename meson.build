project('inform7-ide', 'c', version: '2.0.0', license: 'GPL-3.0-or-later',
    meson_version: '>=0.63.0')

appname = meson.project_name()
app_version = meson.project_version()

gnome = import('gnome')
i18n = import('i18n')

cc = meson.get_compiler('c')

### CHECK FOR PRESENCE OF COMPILER #############################################

# not "required" because we want to print a more informative error message
inform7 = find_program('intools/inform7', required: false)
if not inform7.found()
    error('''I couldn't find the Inform
    compiler or its data files. You must first build the Inform compiler. See
    INSTALL.md for instructions.''')
endif

### DEFINE VARIABLES ###########################################################

datadir = get_option('datadir')
pkgdatadir = datadir / appname
pkglibdir = get_option('libdir') / appname
pkglibexecdir = get_option('libexecdir') / appname
pkgdocsdir = datadir / 'doc' / appname

prefix = get_option('prefix')
abs_datadir = prefix / datadir
abs_pkglibexecdir = prefix / pkglibexecdir
abs_localedir = prefix / datadir / 'locale'

add_project_arguments('-DLOCALEDIR="@0@"'.format(abs_localedir),
    '-DPACKAGE_LOCALE_DIR="@0@"'.format(abs_localedir),
    '-DPACKAGE_DATA_DIR="@0@"'.format(abs_datadir),
    '-DPACKAGE_LIBEXEC_DIR="@0@"'.format(abs_pkglibexecdir),
    '-DG_LOG_DOMAIN="@0@"'.format(appname),
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
handy = dependency('libhandy-1')
gtk = dependency('gtk+-3.0', version: '>=3.24')
gtksourceview = dependency('gtksourceview-4')
gspell = dependency('gspell-1')
goocanvas = dependency('goocanvas-2.0')
webkit = dependency('webkit2gtk-4.1')
plist = dependency('libplist-2.0', required: false)
if not plist.found()
    plist = dependency('libplist')
endif

ratify = dependency('ratify-2', fallback: ['ratify', 'libratify_dep'],
    default_options: ['introspection=disabled', 'gtk_doc=disabled',
        'vapi=disabled'])
libchimara = dependency('chimara', fallback: ['chimara', 'libchimara_dep'],
    default_options: ['bocfel=false', 'nitfol=false', 'player=false',
        'introspection=false', 'plugindir=@0@'.format(pkglibdir)])

### CONFIGURE ##################################################################

config = configuration_data()

config.set_quoted('PACKAGE', appname)
config.set_quoted('PACKAGE_NAME', appname)
config.set_quoted('GETTEXT_PACKAGE', appname)
config.set_quoted('PACKAGE_VERSION', app_version)

# Get current year for updating copyrights
copyright_year = run_command(date, '+%Y', check: true).stdout().strip()
config.set_quoted('COPYRIGHT_YEAR', copyright_year)

# Find Inform version
inform7_output = run_command(inform7, '-version', check: true).stdout()
inform7_version = inform7_output.replace('inform7 version ', '').split()[0]
config.set_quoted('INFORM7_VERSION', inform7_version)

if glib.version().version_compare('>=2.60')
    config.set('GLIB_VERSION_MIN_REQUIRED', 'GLIB_VERSION_2_60')
endif

configure_file(output: 'config.h', configuration: config)

### BUILD AND INSTALL ##########################################################

local_schemas = gnome.compile_schemas()  # Needed for tests

subdir('src')
subdir('po')

install_data('intools/inblorb', 'intools/inform6', 'intools/inform7',
    install_dir: pkglibexecdir, install_mode: 'rwxr-xr-x')

foreach build : ['6L02', '6L38', '6M62']
    install_data('retrospective/@0@/cBlorb'.format(build),
        'retrospective/@0@/ni'.format(build),
        install_dir: pkglibexecdir / 'retrospective' / build,
        install_mode: 'rwxr-xr-x')
endforeach

i18n.merge_file(input: 'com.inform7.IDE.desktop.in',
    output: 'com.inform7.IDE.desktop', type: 'desktop', po_dir: 'po',
    install: true, install_dir: datadir / 'applications')
i18n.merge_file(input: 'data/mime/com.inform7.IDE.xml.in',
    output: 'com.inform7.IDE.xml',
    type: 'xml', po_dir: 'po', data_dirs: meson.current_source_dir() / 'data',
    install: true, install_dir: datadir / 'mime' / 'packages')

install_data('com.inform7.IDE.appdata.xml', install_dir: datadir / 'metainfo')

install_data('com.inform7.IDE.gschema.xml',
    install_dir: datadir / 'glib-2.0' / 'schemas')

install_data('COPYING', 'README.md', install_dir: pkgdocsdir)

install_subdir('data/Extensions', install_dir: pkgdatadir)
install_subdir('data/HTML', install_dir: pkgdatadir)
install_subdir('data/Inter', install_dir: pkgdatadir)
install_subdir('data/Languages', install_dir: pkgdatadir)
install_subdir('data/Miscellany', install_dir: pkgdatadir)
install_subdir('data/Pipelines', install_dir: pkgdatadir)
install_subdir('data/Templates', install_dir: pkgdatadir)
install_subdir('data/highlighting', install_dir: pkgdatadir)
install_subdir('data/styles', install_dir: pkgdatadir,
    exclude_files: ['styles.its', 'styles.pot', 'update-pot.sh'])

retrospective_data_dirs = {
    '6L02': ['Extensions', 'I6T', 'Outcome Pages'],
    '6L38': ['Internal', 'Outcome Pages'],
    '6M62': ['Internal', 'Outcome Pages'],
}
foreach build, internal_dirs : retrospective_data_dirs
    retrospectivedir = pkgdatadir / 'retrospective' / build
    foreach internal_dir : internal_dirs
        install_subdir('retrospective/@0@/@1@'.format(build, internal_dir),
            install_dir: retrospectivedir)
    endforeach
endforeach

# Named Icons
# -----------
# com.inform7.IDE.png: named application icon (inform.iconset)
# com.inform7.IDE.builtin.png: named icon for built-in extension
# com.inform7.IDE.materials.png: named emblem for materials folder
#
# MIME types
# ----------
# com.inform7.IDE.application/x-inform: Inform 7 project (informfile.iconset)
# com.inform7.IDE.application/x-inform-materials: Materials folder (materialsfile.iconset)
# com.inform7.IDE.application/x-inform-skein+xml: Skein file (skeinfile.iconset)
# com.inform7.IDE.text/x-inform: Inform 6 source code (i6file.iconset, inffile.iconset)
# com.inform7.IDE.text/x-natural-inform: Inform 7 source code (i7file.iconset, nifile.iconset)
# com.inform7.IDE.text/x-natural-inform-extension: Extension file (i7xfile.iconset)
# com.inform7.IDE.text/x-blurb: Blurb file (blurbfile.iconset)

icondir = datadir / 'icons' / 'hicolor'
install_subdir('pixmaps/16x16', install_dir: icondir)
install_subdir('pixmaps/32x32', install_dir: icondir)
install_subdir('pixmaps/48x48', install_dir: icondir)
install_subdir('pixmaps/64x64', install_dir: icondir)
install_subdir('pixmaps/128x128', install_dir: icondir)
install_subdir('pixmaps/256x256', install_dir: icondir)

# Script adapted from GNOME Notes's build system
meson.add_install_script('build-aux/meson_post_install.py', abs_datadir)
