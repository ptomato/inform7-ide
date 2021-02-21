subdir('inweb')
subdir('cBlorb')
subdir('inform6')
subdir('inform/licenses')

install_data('ni/ni', install_dir: pkglibexecdir, install_mode: 'rwxr-xr-x')

generate_resource_xml = find_program('generate-resource-xml.sh')
generated_gresource_xml = configure_file(input: inform_tarball,
    output: 'com.inform7.IDE.generated.gresource.xml',
    command: [generate_resource_xml, inform_tarball, '@OUTPUT@'])
resources_generated = gnome.compile_resources('resources-generated',
    generated_gresource_xml, c_name: 'i7g',
    source_dir: meson.current_build_dir())

resources = gnome.compile_resources('resources',
    'com.inform7.IDE.gresource.xml', dependencies: license_html, c_name: 'i7',
    source_dir: meson.current_build_dir())

gui = static_library('inform7gui', 'actions.c', 'app.c', 'app-colorscheme.c',
    'builder.c', 'configfile.c', 'document.c', 'document-search.c', 'elastic.c',
    'error.c', 'extension.c', 'file.c', 'history.c', 'html.c', 'lang.c',
    'newdialog.c', 'node.c', 'notepad.c', 'panel.c', 'prefs.c',
    'searchwindow.c', 'skein.c', 'skein-view.c', 'source-view.c', 'spawn.c',
    'story.c', 'story-compile.c', 'story-game.c', 'story-index.c',
    'story-results.c', 'story-settings.c', 'story-skein.c', 'story-source.c',
    'story-transcript.c', 'transcript-diff.c', 'transcript-renderer.c',
    'welcomedialog.c', resources, resources_generated,
    include_directories: top_include,
    dependencies: [libm, libxml, glib, gio, gdk, gtk, gtksourceview, gtkspell,
        goocanvas, plist, webkit, ratify, libchimara])

executable('gnome-inform7', 'main.c', export_dynamic: true,
    include_directories: '..', dependencies: [glib, gtk, gtksourceview],
    link_whole: gui, install: true, install_dir: get_option('bindir'))

test_data_dir = meson.current_source_dir() / 'tests'
test_inform7 = executable('test-inform7', 'tests/test.c', 'tests/app-test.c',
    'tests/skein-test.c', 'tests/story-test.c',
    include_directories: top_include,
    c_args: '-DTEST_DATA_DIR="@0@"'.format(test_data_dir),
    dependencies: [glib, gtk, gtksourceview, goocanvas], link_whole: gui)

# Skip the /app/extensions and /story tests, because they are too tightly
# coupled to the installed files, and and will fail.
skip_paths = [
    '-s=/app/extensions/install-remove',
    '-s=/app/extensions/get-builtin',
    '-s=/app/extensions/get-version',
    '-s=/app/extensions/case-insensitive',
    '-s=/story/materials-file',
    '-s=/story/old-materials-file',
    '-s=/story/renames-materials-file',
]
test_env = environment()
test_env.append('LD_LIBRARY_PATH',
    meson.build_root() / 'subprojects' / 'ratify',
    meson.build_root() / 'subprojects' / 'chimara' / 'libchimara')
test_env.append('GSETTINGS_SCHEMA_DIR', meson.build_root())
test_env.set('GSETTINGS_BACKEND', 'memory')
test_env.set('NO_AT_BRIDGE', '1')
test_env.set('GNOME_INFORM_DATA_DIR', meson.source_root() / 'data')
test_env.set('GNOME_INFORM_LIBEXEC_DIR', meson.current_source_dir() / 'ni')
test('inform7', test_inform7, protocol: 'tap', args: ['--tap'] + skip_paths,
    env: test_env, depends: local_schemas)
