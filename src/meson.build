subdir('inweb')
subdir('cBlorb')
subdir('inform6')
subdir('inform/licenses')

install_data('ni/ni', install_dir: pkglibexecdir, install_mode: 'rwxr-xr-x')

osxcart_sources = [
    'osxcart/osxcart/init.c',
    'osxcart/osxcart/plist.c',
    'osxcart/osxcart/plist-write.c',
    'osxcart/osxcart/plist-parser.c',
    'osxcart/osxcart/rtf.c',
    'osxcart/osxcart/rtf-serialize.c',
    'osxcart/osxcart/rtf-deserialize.c',
    'osxcart/osxcart/rtf-document.c',
    'osxcart/osxcart/rtf-fonttbl.c',
    'osxcart/osxcart/rtf-colortbl.c',
    'osxcart/osxcart/rtf-ignore.c',
    'osxcart/osxcart/rtf-footnote.c',
    'osxcart/osxcart/rtf-picture.c',
    'osxcart/osxcart/rtf-field.c',
    'osxcart/osxcart/rtf-stylesheet.c',
    'osxcart/osxcart/rtf-langcode.c',
    'osxcart/osxcart/rtf-state.c',
]
osxcart = static_library('osxcart', osxcart_sources,
    include_directories: [top_include, 'osxcart'],
    c_args: '-DG_LOG_DOMAIN="Osxcart"',
    dependencies: [glib, gio, gdk, gdkpixbuf, gtk])

chimara_sources = [
    'chimara/libchimara/abort.c',
    'chimara/libchimara/case.c',
    'chimara/libchimara/charset.c',
    'chimara/libchimara/chimara-glk.c',
    'chimara/libchimara/chimara-if.c',
    'chimara/libchimara/datetime.c',
    'chimara/libchimara/dispatch.c',
    'chimara/libchimara/event.c',
    'chimara/libchimara/fileref.c',
    'chimara/libchimara/garglk.c',
    'chimara/libchimara/gestalt.c',
    'chimara/libchimara/gi_blorb.c',
    'chimara/libchimara/gi_dispa.c',
    'chimara/libchimara/glk.c',
    'chimara/libchimara/glkunix.c',
    'chimara/libchimara/graphics.c',
    'chimara/libchimara/hyperlink.c',
    'chimara/libchimara/init.c',
    'chimara/libchimara/input.c',
    'chimara/libchimara/magic.c',
    'chimara/libchimara/mouse.c',
    'chimara/libchimara/pager.c',
    'chimara/libchimara/resource.c',
    'chimara/libchimara/schannel.c',
    'chimara/libchimara/stream.c',
    'chimara/libchimara/strio.c',
    'chimara/libchimara/style.c',
    'chimara/libchimara/timer.c',
    'chimara/libchimara/ui-buffer.c',
    'chimara/libchimara/ui-graphics.c',
    'chimara/libchimara/ui-grid.c',
    'chimara/libchimara/ui-message.c',
    'chimara/libchimara/ui-misc.c',
    'chimara/libchimara/ui-style.c',
    'chimara/libchimara/ui-textwin.c',
    'chimara/libchimara/ui-window.c',
    'chimara/libchimara/window.c',
]
chimara_marshallers = gnome.genmarshal('chimara-marshallers',
    sources: 'chimara/libchimara/chimara-marshallers.txt',
    prefix: '_chimara_marshal')
chimara = static_library('chimara', chimara_sources, chimara_marshallers,
    include_directories: [top_include, 'chimara'],
    c_args: ['-DG_LOG_DOMAIN="Chimara"',
        '-DPLUGINDIR="@0@"'.format(abs_plugindir)],
    dependencies: [glib, gthread, gmodule, gio, pango, gtk, gstreamer])

frotz_sources = [
    'chimara/interpreters/frotz/buffer.c',
    'chimara/interpreters/frotz/err.c',
    'chimara/interpreters/frotz/fastmem.c',
    'chimara/interpreters/frotz/files.c',
    'chimara/interpreters/frotz/input.c',
    'chimara/interpreters/frotz/main.c',
    'chimara/interpreters/frotz/math.c',
    'chimara/interpreters/frotz/object.c',
    'chimara/interpreters/frotz/process.c',
    'chimara/interpreters/frotz/quetzal.c',
    'chimara/interpreters/frotz/random.c',
    'chimara/interpreters/frotz/redirect.c',
    'chimara/interpreters/frotz/sound.c',
    'chimara/interpreters/frotz/stream.c',
    'chimara/interpreters/frotz/table.c',
    'chimara/interpreters/frotz/text.c',
    'chimara/interpreters/frotz/variable.c',
    'chimara/interpreters/frotz/glkscreen.c',
    'chimara/interpreters/frotz/glkmisc.c',
]
frotz_extraflags = cc.get_supported_arguments(['-Wno-unused'])
shared_module('frotz', frotz_sources, name_prefix: '', c_args: frotz_extraflags,
    include_directories: ['chimara/libchimara', 'chimara/interpreters/frotz'],
    install: true, install_dir: pkglibdir)

frotz_docs = [
    'chimara/interpreters/frotz/AUTHORS',
    'chimara/interpreters/frotz/COPYING',
    'chimara/interpreters/frotz/README',
    'chimara/interpreters/frotz/TODO',
]
install_data(frotz_docs, install_dir: pkgdocsdir / 'frotz')

glulxe_sources = [
    'chimara/interpreters/glulxe/accel.c',
    'chimara/interpreters/glulxe/exec.c',
    'chimara/interpreters/glulxe/files.c',
    'chimara/interpreters/glulxe/float.c',
    'chimara/interpreters/glulxe/funcs.c',
    'chimara/interpreters/glulxe/gestalt.c',
    'chimara/interpreters/glulxe/glkop.c',
    'chimara/interpreters/glulxe/heap.c',
    'chimara/interpreters/glulxe/main.c',
    'chimara/interpreters/glulxe/operand.c',
    'chimara/interpreters/glulxe/osdepend.c',
    'chimara/interpreters/glulxe/profile.c',
    'chimara/interpreters/glulxe/search.c',
    'chimara/interpreters/glulxe/serial.c',
    'chimara/interpreters/glulxe/string.c',
    'chimara/interpreters/glulxe/unixstrt.c',
    'chimara/interpreters/glulxe/vm.c',
]
glulxe_extraflags = cc.get_supported_arguments(['-Wno-uninitialized',
    '-Wno-unused', '-Wno-strict-aliasing'])
shared_module('glulxe', glulxe_sources, name_prefix: '',
    c_args: ['-DOS_UNIX', glulxe_extraflags],
    include_directories: ['chimara/libchimara'],
    install: true, install_dir: pkglibdir)

install_data('chimara/interpreters/glulxe/README',
    install_dir: pkgdocsdir / 'glulxe')

# Automatically generate version.h
git_version = configuration_data({
    'GIT_MAJOR': 1,
    'GIT_MINOR': 2,
    'GIT_PATCH': 9,
})
git_version_h = configure_file(configuration: git_version,
    output: 'version.h')

git_sources = [
    'chimara/interpreters/git/compiler.c',
    'chimara/interpreters/git/gestalt.c',
    'chimara/interpreters/git/git.c',
    'chimara/interpreters/git/git_unix.c',
    'chimara/interpreters/git/glkop.c',
    'chimara/interpreters/git/heap.c',
    'chimara/interpreters/git/memory.c',
    'chimara/interpreters/git/opcodes.c',
    'chimara/interpreters/git/operands.c',
    'chimara/interpreters/git/peephole.c',
    'chimara/interpreters/git/savefile.c',
    'chimara/interpreters/git/saveundo.c',
    'chimara/interpreters/git/search.c',
    'chimara/interpreters/git/terp.c',
    'chimara/interpreters/git/accel.c',
]
git_extraflags = cc.get_supported_arguments(['-Wno-unused'])
shared_module('git', git_sources, git_version_h, name_prefix: '',
    c_args: git_extraflags,
    include_directories: ['chimara/libchimara', 'chimara/interpreters/git'],
    install: true, install_dir: pkglibdir)

install_data('chimara/interpreters/git/README.txt',
    install_dir: pkgdocsdir / 'git')

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
    include_directories: [top_include, 'osxcart', 'chimara'],
    dependencies: [libm, libxml, uuid, glib, gio, gdk, gtk, gtksourceview,
        gtkspell, goocanvas, webkit])

executable('gnome-inform7', 'main.c', export_dynamic: true,
    include_directories: '..', dependencies: [glib, gtk, gtksourceview],
    link_with: osxcart, link_whole: [chimara, gui],
    install: true, install_dir: get_option('bindir'))

test_data_dir = meson.current_source_dir() / 'tests'
test_inform7 = executable('test-inform7', 'tests/test.c', 'tests/app-test.c',
    'tests/skein-test.c', 'tests/story-test.c',
    include_directories: [top_include, 'osxcart', 'chimara'],
    c_args: '-DTEST_DATA_DIR="@0@"'.format(test_data_dir),
    dependencies: [glib, gtk, gtksourceview, goocanvas],
    link_with: osxcart, link_whole: [chimara, gui])

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
test_env.append('GSETTINGS_SCHEMA_DIR', meson.build_root())
test_env.set('GSETTINGS_BACKEND', 'memory')
test_env.set('NO_AT_BRIDGE', '1')
test_env.set('GNOME_INFORM_DATA_DIR', meson.source_root() / 'data')
test_env.set('GNOME_INFORM_LIBEXEC_DIR', meson.current_source_dir() / 'ni')
test('inform7', test_inform7, protocol: 'tap', args: ['--tap'] + skip_paths,
    env: test_env, depends: local_schemas)
