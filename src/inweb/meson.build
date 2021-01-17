inweb_sources = [
    'Contents.w',
    'Preliminaries/Inweb vs Cweb.w',
    'Preliminaries/The inweb Manual.w',
    'Chapter 1/Basics.w',
    'Chapter 1/Configuration.w',
    'Chapter 1/Program Control.w',
    'Chapter 2/Bibliographic Data.w',
    'Chapter 2/Line Categories.w',
    'Chapter 2/Paragraph Numbering.w',
    'Chapter 2/Reading Sections.w',
    'Chapter 2/The Parser.w',
    'Chapter 3/The Analyser.w',
    'Chapter 3/The Swarm.w',
    'Chapter 3/The Tangler.w',
    'Chapter 3/The Weaver.w',
    'Chapter 4/C for Inform.w',
    'Chapter 4/C-Like Languages.w',
    'Chapter 4/Programming Languages.w',
    'Chapter 5/HTML Formats.w',
    'Chapter 5/Plain Text Format.w',
    'Chapter 5/Running Through TeX.w',
    'Chapter 5/TeX Format.w',
    'Chapter 5/Weave Formats.w',
]

inweb_extraflags = cc.get_supported_arguments(['-Wno-unused-but-set-variable',
    '-Wno-unknown-pragmas', '-Wno-tautological-compare', '-Wno-array-bounds',
    '-Wno-format-overflow', '-Wno-stringop-overflow'])

inweb_bootstrap = executable('inweb-bootstrap', 'Tangled/inweb.c',
    c_args: inweb_extraflags)

webdir = meson.current_source_dir() / '..'
inweb_tangled = custom_target('inweb2.c', depend_files: inweb_sources,
    output: 'inweb2.c',
    command: [inweb_bootstrap, meson.current_source_dir(), '-tangle',
        '-tangle-to', '@OUTPUT@', '-at', webdir, '-import-from', webdir])

inweb = executable('inweb', inweb_tangled, c_args: inweb_extraflags)
