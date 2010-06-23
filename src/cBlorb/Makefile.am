# This line won't be necessary anymore in Automake 1.10a
pkglibexecdir = $(libexecdir)/$(PACKAGE)

cblorbsrc = \
	Contents.w \
	Preliminaries/The\ cblorb\ Manual.w \
	Chapter\ 1/Blurb\ Parser.w \
	Chapter\ 1/Main.w \
	Chapter\ 1/Memory.w \
	Chapter\ 1/Text\ Files.w \
	Chapter\ 2/Blorb\ Writer.w \
	Chapter\ 3/Base64.w \
	Chapter\ 3/Links\ and\ Auxiliary\ Files.w \
	Chapter\ 3/Placeholders.w \
	Chapter\ 3/Releaser.w \
	Chapter\ 3/Solution\ Deviser.w \
	Chapter\ 3/Templates.w \
	Chapter\ 3/Website\ Maker.w
cblorbmaterials = Materials/cover-sheet.tex

pkglibexec_PROGRAMS = cBlorb
nodist_cBlorb_SOURCES = Tangled/cBlorb.c
BUILT_SOURCES = $(nodist_cBlorb_SOURCES)
$(nodist_cBlorb_SOURCES) : $(cblorbsrc)
	$(PERL) ../inweb/Tangled/inweb.pl . -tangle -at ../
CLEANFILES = $(nodist_cBlorb_SOURCES)

if MANUALS
# The cBlorb manual
cblorbmanual = Woven/Complete.pdf
# The various extra parts of the manual that can be built
cblorbwebsite = Woven/index.html Woven/download.gif Woven/lemons.jpg
cblorbchapters = \
	Woven/Complete.pdf \
	Woven/Preliminaries.pdf \
	Woven/Chapter-1.pdf \
	Woven/Chapter-2.pdf \
	Woven/Chapter-3.pdf
cblorbsections = \
	Woven/P-man.pdf \
	Woven/1-blurb.pdf \
	Woven/1-main.pdf \
	Woven/1-mem.pdf \
	Woven/1-text.pdf \
	Woven/2-blorb.pdf \
	Woven/3-links.pdf \
	Woven/3-place.pdf \
	Woven/3-rel.pdf \
	Woven/3-sol.pdf \
	Woven/3-templ.pdf \
	Woven/3-web.pdf

# What to build and how to build it
if MANUALS_SECTIONS
cblorbweavefiles = $(cblorbwebsite) $(cblorbchapters) $(cblorbsections)
inwebcommand = sections
else
if MANUALS_CHAPTERS
cblorbweavefiles = $(cblorbwebsite) $(cblorbchapters)
inwebcommand = chapters
else
cblorbweavefiles = 
inwebcommand = all # Slightly paradoxical, that...
endif
endif

cblorbdocdir = $(datadir)/doc/$(PACKAGE)/cBlorb
cblorbdoc_DATA = $(cblorbmanual) $(cblorbweavefiles)

$(cblorbweavefiles): $(cblorbmanual)
$(cblorbmanual): $(cblorbsrc) Makefile
	$(PERL) ../inweb/Tangled/inweb.pl . -closed -weave $(inwebcommand) -at ../

CLEANFILES += $(cblorbmanual) $(cblorbweavefiles)
endif

EXTRA_DIST = $(cblorbmaterials)

dist-hook:
	cp --preserve=mode,ownership,timestamps --parents $(cblorbsrc) $(distdir)
	$(MKDIR_P) $(distdir)/Woven $(distdir)/Tangled

