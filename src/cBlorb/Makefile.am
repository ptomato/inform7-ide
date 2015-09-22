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

INWEB = $(top_builddir)/src/inweb/Tangled/inweb -at $(top_srcdir)/src/
# Nifty silent rules strings
tangle_verbose = $(tangle_verbose_@AM_V@)
tangle_verbose_ = $(tangle_verbose_@AM_DEFAULT_V@)
tangle_verbose_0 = @echo "  TANGLE  " $@;
weave_verbose = $(weave_verbose_@AM_V@)
weave_verbose_ = $(weave_verbose_@AM_DEFAULT_V@)
weave_verbose_0 = @echo "  WEAVE   " $@;

pkglibexec_PROGRAMS = cBlorb
nodist_cBlorb_SOURCES = Tangled/cBlorb.c
BUILT_SOURCES = $(nodist_cBlorb_SOURCES)
$(nodist_cBlorb_SOURCES) : $(cblorbsrc)
	$(tangle_verbose)$(MKDIR_P) Tangled && \
	$(INWEB) $(srcdir) -tangle -tangle-to $(nodist_cBlorb_SOURCES)
CLEANFILES = $(nodist_cBlorb_SOURCES)

if MANUALS
# The cBlorb manual
cblorbmanual = Woven/Complete.html
cblorbmanualfiles = Woven/inweb.css Woven/crumbs.gif
# The expanded "website" version of the manual that can be built
cblorbsections = \
	Woven/index.html \
	Woven/download.gif \
	Woven/lemons.jpg \
	Woven/Complete.html \
	Woven/Preliminaries.html \
	Woven/P-man.html \
	Woven/Chapter-1.html \
	Woven/1-blurb.html \
	Woven/1-main.html \
	Woven/1-mem.html \
	Woven/1-text.html \
	Woven/Chapter-2.html \
	Woven/2-blorb.html \
	Woven/Chapter-3.html \
	Woven/3-b64.html \
	Woven/3-links.html \
	Woven/3-place.html \
	Woven/3-rel.html \
	Woven/3-sol.html \
	Woven/3-templ.html \
	Woven/3-web.html \
	$(NULL)

# What to build and how to build it
if MANUALS_SECTIONS
cblorbweavefiles = $(cblorbmanualfiles) $(cblorbsections)
inwebcommand = sections
else
cblorbweavefiles = $(cblorbmanualfiles)
inwebcommand =
endif

cblorbdocdir = $(datadir)/doc/$(PACKAGE)/cBlorb
cblorbdoc_DATA = $(cblorbmanual) $(cblorbweavefiles)

$(cblorbweavefiles): $(cblorbmanual)
$(cblorbmanual): $(cblorbsrc) $(cblorbmaterials) Makefile
	$(weave_verbose)$(MKDIR_P) --mode=u+w,o-w $(builddir)/Woven && \
	$(INWEB) $(srcdir) -closed -weave $(inwebcommand) -format html

CLEANFILES += $(cblorbmanual) $(cblorbweavefiles)
endif

EXTRA_DIST = $(cblorbmaterials)

dist-hook:
	export ABSDISTDIR=`readlink -f $(distdir)`; \
	$(MKDIR_P) --mode=u+w \
		$(distdir)/Preliminaries \
		$(distdir)/Chapter\ 1 \
		$(distdir)/Chapter\ 2 \
		$(distdir)/Chapter\ 3 \
		$(distdir)/Woven \
		$(distdir)/Tangled \
		&& \
	cd $(srcdir) && \
	cp --preserve=mode,ownership,timestamps --parents \
		$(cblorbsrc) "$$ABSDISTDIR"

GITIGNOREFILES = Woven

-include $(top_srcdir)/git.mk
