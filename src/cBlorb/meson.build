cblorb_sources = [
    'Contents.w',
    'Preliminaries/The cblorb Manual.w',
    'Chapter 1/Blurb Parser.w',
    'Chapter 1/Main.w',
    'Chapter 1/Memory.w',
    'Chapter 1/Text Files.w',
    'Chapter 2/Blorb Writer.w',
    'Chapter 3/Base64.w',
    'Chapter 3/Links and Auxiliary Files.w',
    'Chapter 3/Placeholders.w',
    'Chapter 3/Releaser.w',
    'Chapter 3/Solution Deviser.w',
    'Chapter 3/Templates.w',
    'Chapter 3/Website Maker.w',
]

cblorb_extraflags = cc.get_supported_arguments(['-Wno-unused-but-set-variable',
    '-Wno-format-overflow'])

webdir = meson.current_source_dir() / '..'
cblorb_tangled = custom_target('cBlorb.c', depend_files: cblorb_sources,
    output: 'cBlorb.c',
    command: [inweb, '-at', webdir, meson.current_source_dir(), '-tangle',
        '-tangle-to', '@OUTPUT@'])
executable('cBlorb', cblorb_tangled, c_args: cblorb_extraflags, install: true,
    install_dir: pkglibexecdir)
