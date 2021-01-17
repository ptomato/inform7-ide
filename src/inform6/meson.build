inform6_extraflags = cc.get_supported_arguments(['-Wno-format-overflow',
    '-Wno-unused', '-Wno-maybe-uninitialized', '-Wno-misleading-indentation',
    '-Wno-implicit-function-declaration'])

executable('inform6', 'arrays.c', 'asm.c', 'bpatch.c', 'chars.c', 'directs.c',
    'errors.c', 'expressc.c', 'expressp.c', 'files.c', 'inform.c', 'lexer.c',
    'linker.c', 'memory.c', 'objects.c', 'states.c', 'symbols.c', 'syntax.c',
    'tables.c', 'text.c', 'veneer.c', 'verbs.c',
    c_args: ['-ansi', '-DLINUX', inform6_extraflags],
    install: true, install_dir: pkglibexecdir)

install_data('readme.txt', 'licence.txt', 'DebugFileFormat.txt',
    'ReleaseNotes.html',
    install_dir: pkgdocsdir / 'inform6')
