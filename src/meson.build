subdir('inform/licenses')

generate_resource_xml = find_program('generate-resource-xml.py')
# The "input" argument is bogus here. This is a workaround for
# https://github.com/mesonbuild/meson/issues/10367
generated_gresource_xml = configure_file(
    input: generate_resource_xml.full_path(),
    output: 'com.inform7.IDE.generated.gresource.xml',
    command: [generate_resource_xml, meson.current_source_dir(), '@OUTPUT@'])
resources_generated = gnome.compile_resources('resources-generated',
    generated_gresource_xml, c_name: 'i7g')

resources = gnome.compile_resources('resources',
    'com.inform7.IDE.gresource.xml', dependencies: license_html, c_name: 'i7',
    source_dir: [meson.current_build_dir(),
        meson.source_root() / 'retrospective'])

gui = static_library('inform7gui', 'actions.c', 'app.c', 'app-colorscheme.c',
    'app-retrospective.c', 'blob.c', 'builder.c', 'configfile.c', 'document.c',
    'elastic.c', 'error.c', 'extension.c', 'file.c', 'history.c', 'html.c',
    'lang.c', 'newdialog.c', 'node.c', 'notepad.c', 'panel.c', 'prefs.c',
    'project-settings.c', 'searchbar.c', 'searchwindow.c', 'skein.c',
    'skein-view.c', 'source-view.c', 'spawn.c', 'story.c', 'story-compile.c',
    'story-game.c', 'story-index.c', 'story-results.c', 'story-settings.c',
    'story-skein.c', 'story-source.c', 'story-transcript.c', 'toast.c',
    'transcript-diff.c', 'transcript-entry.c', 'uri-scheme.c',
    'welcomedialog.c',
    resources, resources_generated,
    include_directories: top_include,
    dependencies: [libm, libxml, glib, gio, gdk, gtk, gtksourceview, gspell,
        goocanvas, handy, plist, webkit, ratify, libchimara])

# install_rpath allows running from a non-system install path
executable('inform7-ide', 'main.c', export_dynamic: true,
    include_directories: '..', dependencies: [glib, gtk, gtksourceview],
    link_whole: gui, install: true, install_dir: get_option('bindir'),
    install_rpath: get_option('prefix') / get_option('libdir'))

test_inform7 = executable('test-inform7', 'tests/app-test.c',
    'tests/blob-test.c', 'tests/difftest.c', 'tests/skein-test.c',
    'tests/story-test.c', 'tests/test.c',
    include_directories: top_include,
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
test_env.set('G_TEST_SRCDIR', meson.current_source_dir())
test_env.set('G_TEST_BUILDDIR', meson.current_build_dir())
test_env.set('INFORM7_IDE_DATA_DIR', meson.project_source_root() / 'data')
test_env.set('INFORM7_IDE_LIBEXEC_DIR', meson.project_source_root() / 'intools')
test('inform7', test_inform7, protocol: 'tap', args: ['--tap'] + skip_paths,
    env: test_env, depends: local_schemas)
