inwebsrc = \
	Contents.w \
	Preliminaries/The\ inweb\ Manual.w \
	Chapter\ 1/Command\ Line\ and\ Errors.w \
	Chapter\ 1/Program\ Control.w \
	Chapter\ 2/Identifiers.w \
	Chapter\ 2/Line\ Categories.w \
	Chapter\ 2/Reading\ Sections.w \
	Chapter\ 2/The\ Parser.w \
	Chapter\ 3/Backus-Naur\ Form.w \
	Chapter\ 3/C\ for\ Inform.w \
	Chapter\ 3/Programming\ Languages.w \
	Chapter\ 3/The\ Analyser.w \
	Chapter\ 3/The\ Swarm.w \
	Chapter\ 3/The\ Tangler.w \
	Chapter\ 3/The\ Weaver.w
inwebmaterials = \
	Materials/download.gif \
	Materials/cover-sheet.tex \
	Materials/lemons.jpg \
	Materials/chaptered-index.html \
	Materials/unchaptered-index.html \
	Materials/Contents.w \
	Materials/Main.w \
	Materials/inweb-macros.tex \
	Materials/inweb-configuration.txt

dist_noinst_SCRIPTS = Tangled/inweb.pl
$(dist_noinst_SCRIPTS): $(inwebsrc)
	$(PERL) Tangled/inweb.pl . -tangle -at ../
# Do not add inweb.pl to CLEANFILES, as then there would be no more inweb to
# build inweb with!

if MANUALS
# The inweb manual
inwebmanual = Woven/Complete.pdf
# The various extra parts of the manual that can be built
inwebsite = Woven/index.html Woven/download.gif Woven/lemons.jpg
inwebchapters = \
	Woven/Preliminaries.pdf \
	Woven/Chapter-1.pdf \
	Woven/Chapter-2.pdf \
	Woven/Chapter-3.pdf
inwebsections = \
	Woven/P-man.pdf \
	Woven/1-cli.pdf \
	Woven/1-pc.pdf \
	Woven/2-ident.pdf \
	Woven/2-lcats.pdf \
	Woven/2-parse.pdf \
	Woven/2-read.pdf \
	Woven/3-anal.pdf \
	Woven/3-bnf.pdf \
	Woven/3-cfori.pdf \
	Woven/3-plan.pdf \
	Woven/3-swarm.pdf \
	Woven/3-tang.pdf \
	Woven/3-weave.pdf

# What to build and how to build it
if MANUALS_SECTIONS
inwebweavefiles = $(inwebsite) $(inwebchapters) $(inwebsections)
inwebcommand = sections
else
if MANUALS_CHAPTERS
inwebweavefiles = $(inwebsite) $(inwebchapters)
inwebcommand = chapters
else
inwebweavefiles = 
inwebcommand = all # Slightly paradoxical, that...
endif
endif

noinst_DATA = $(inwebmanual) $(inwebweavefiles)
$(inwebweavefiles): $(inwebmanual)

# Make sure that the last doc build is newer than the Makefile, otherwise the
# user could have switched from 'chapters' to 'sections' for example and the
# extra PDF files wouldn't build
$(inwebmanual): $(inwebsrc) Makefile
	$(PERL) Tangled/inweb.pl . -closed -weave $(inwebcommand) -at ../

CLEANFILES = $(inwebweavefiles) $(inwebmanual)
endif

DISTCLEANFILES = Materials/inweb-configuration.txt

EXTRA_DIST = $(inwebmaterials)

# Copy the sources ourselves, they have SPACES in their filenames! Wooooo!
# I'm automake! Spaces are evil! I refuse to handle such newfangled notions!
dist-hook:
	cp --preserve=mode,ownership,timestamps --parents $(inwebsrc) $(distdir)
	$(MKDIR_P) $(distdir)/Woven

