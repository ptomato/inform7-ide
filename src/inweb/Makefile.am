AM_CFLAGS = $(INWEB_EXTRAFLAGS)

inwebsrc = \
	Contents.w \
	Preliminaries/Inweb\ vs\ Cweb.w \
	Preliminaries/The\ inweb\ Manual.w \
	Chapter\ 1/Basics.w  \
	Chapter\ 1/Configuration.w \
	Chapter\ 1/Program\ Control.w \
	Chapter\ 2/Bibliographic\ Data.w \
	Chapter\ 2/Line\ Categories.w \
	Chapter\ 2/Paragraph\ Numbering.w \
	Chapter\ 2/Reading\ Sections.w \
	Chapter\ 2/The\ Parser.w \
	Chapter\ 3/The\ Analyser.w \
	Chapter\ 3/The\ Swarm.w \
	Chapter\ 3/The\ Tangler.w \
	Chapter\ 3/The\ Weaver.w \
	Chapter\ 4/C\ for\ Inform.w \
	Chapter\ 4/C-Like\ Languages.w \
	Chapter\ 4/Programming\ Languages.w \
	Chapter\ 5/HTML\ Formats.w \
	Chapter\ 5/Plain\ Text\ Format.w \
	Chapter\ 5/Running\ Through\ TeX.w \
	Chapter\ 5/TeX\ Format.w \
	Chapter\ 5/Weave\ Formats.w \
	$(NULL)
inwebmaterials = \
	Materials/chaptered-index.html \
	Materials/chaptered-tex-index.html \
	Materials/Contents.w \
	Materials/cover-sheet.html \
	Materials/cover-sheet.tex \
	Materials/cover-sheet.txt \
	Materials/crumbs.gif \
	Materials/download.gif \
	Materials/epub-index.html \
	Materials/epub-template.html \
	Materials/inweb-configuration.txt \
	Materials/inweb.css \
	Materials/inweb-macros.tex \
	Materials/lemons.jpg \
	Materials/Main.w \
	Materials/template.html \
	Materials/unchaptered-index.html \
	Materials/unchaptered-tex-index.html \
	$(NULL)

# Nifty silent rules strings
tangle_verbose = $(tangle_verbose_@AM_V@)
tangle_verbose_ = $(tangle_verbose_@AM_DEFAULT_V@)
tangle_verbose_0 = @echo "  TANGLE  " $@;
weave_verbose = $(weave_verbose_@AM_V@)
weave_verbose_ = $(weave_verbose_@AM_DEFAULT_V@)
weave_verbose_0 = @echo "  WEAVE   " $@;

noinst_PROGRAMS = Tangled/inweb-bootstrap Tangled/inweb
Tangled_inweb_bootstrap_SOURCES = Tangled/inweb.c
Tangled_inweb_SOURCES = Tangled/inweb2.c
BUILT_SOURCES = $(Tangled_inweb_SOURCES)
WEBDIR = $(srcdir)/..
INWEB = Tangled/inweb -at $(WEBDIR)/ -import-from $(WEBDIR)/
$(Tangled_inweb_SOURCES): $(inwebsrc) Tangled/inweb-bootstrap
	$(tangle_verbose)Tangled/inweb-bootstrap $(WEBDIR)/inweb -tangle \
		-tangle-to $(Tangled_inweb_SOURCES) \
		-at $(WEBDIR)/ -import-from $(WEBDIR)/
CLEANFILES = $(BUILT_SOURCES) Tangled/win32.h
# Do not add Tangled/inweb.c to CLEANFILES, as then there would be no more inweb
# to build inweb with!

if MANUALS
# The inweb manual
inwebmanual = Woven/Complete.html
inwebmanualfiles = Woven/inweb.css Woven/crumbs.gif
# The expanded "website" version of the manual that can be built
inwebsections = \
	Woven/index.html \
	Woven/download.gif \
	Woven/lemons.jpg \
	Woven/Preliminaries.html \
	Woven/P-cweb.html \
	Woven/P-man.html \
	Woven/Chapter-1.html \
	Woven/1-basic.html \
	Woven/1-match.html \
	Woven/1-mem.html \
	Woven/1-text.html \
	Woven/Chapter-2.html \
	Woven/2-cli.html \
	Woven/2-errs.html \
	Woven/2-pc.html \
	Woven/Chapter-3.html \
	Woven/3-bibl.html \
	Woven/3-lcats.html \
	Woven/3-parse.html \
	Woven/3-pnum.html \
	Woven/3-read.html \
	Woven/Chapter-4.html \
	Woven/4-anal.html \
	Woven/4-swarm.html \
	Woven/4-tang.html \
	Woven/4-weave.html \
	Woven/Chapter-5.html \
	Woven/5-cfori.html \
	Woven/5-clike.html \
	Woven/5-plan.html \
	Woven/Chapter-6.html \
	Woven/6-form.html \
	Woven/6-html.html \
	Woven/6-plain.html \
	Woven/6-rtex.html \
	Woven/6-tex.html \
	$(NULL)

# What to build and how to build it
if MANUALS_SECTIONS
inwebweavefiles = $(inwebmanualfiles) $(inwebsections)
inwebcommand = sections
else
inwebweavefiles = $(inwebmanualfiles)
inwebcommand =
endif

noinst_DATA = $(inwebmanual) $(inwebweavefiles)
$(inwebweavefiles): $(inwebmanual)

# Make sure that the last doc build is newer than the Makefile, otherwise the
# user could have switched from the whole manual to the sections manual, for
# example, and the extra files wouldn't build
$(inwebmanual): $(inwebsrc) $(inwebmaterials) Makefile Tangled/inweb
	$(weave_verbose)$(MKDIR_P) --mode=u+w,o-w $(builddir)/Woven;
	$(INWEB) $(WEBDIR)/inweb -closed -weave $(inwebcommand) -format html

CLEANFILES += $(inwebweavefiles) $(inwebmanual)
endif

DISTCLEANFILES = Materials/inweb-configuration.txt

EXTRA_DIST = $(inwebmaterials)

# Copy the sources ourselves, they have SPACES in their filenames! Wooooo!
# I'm automake! Spaces are evil! I refuse to handle such newfangled notions!
#
# Also: why should the distdir be unwritable?!
#
# Also: why is distdir sometimes an absolute path and sometimes relative?
dist-hook:
	export ABSDISTDIR=`readlink -f $(distdir)`; \
	$(MKDIR_P) --mode=u+w \
		$(distdir)/Preliminaries \
		$(distdir)/Chapter\ 1 \
		$(distdir)/Chapter\ 2 \
		$(distdir)/Chapter\ 3 \
		$(distdir)/Chapter\ 4 \
		$(distdir)/Chapter\ 5 \
		$(distdir)/Chapter\ 6 \
		$(distdir)/Figures \
		$(distdir)/Woven \
		&& \
	cd $(srcdir) && \
	cp --preserve=mode,ownership,timestamps --parents \
		$(inwebsrc) "$$ABSDISTDIR"

GITIGNOREFILES = Woven

-include $(top_srcdir)/git.mk
