inlibsrc = \
	Contents.w \
	Chapter\ 1/Inlib.w \
	Chapter\ 1/Platform-Specific\ Definitions.w \
	Chapter\ 2/Command\ Line\ Arguments.w \
	Chapter\ 2/C\ Strings.w \
	Chapter\ 2/Debugging\ Log.w \
	Chapter\ 2/Error\ Messages.w \
	Chapter\ 2/Memory.w \
	Chapter\ 2/Streams.w \
	Chapter\ 2/String\ Manipulation.w \
	Chapter\ 3/Case-Insensitive\ Filenames.w \
	Chapter\ 3/Directories.w \
	Chapter\ 3/Filenames.w \
	Chapter\ 3/Pathnames.w \
	Chapter\ 3/Shell.w \
	Chapter\ 4/Characters.w \
	Chapter\ 4/ISO\ Pattern\ Matching.w \
	Chapter\ 4/Pattern\ Matching.w \
	Chapter\ 4/Text\ Files.w \
	Chapter\ 5/Epub\ Ebooks.w \
	Chapter\ 5/HTML.w \
	Chapter\ 6/Binary\ Files.w \
	Chapter\ 6/Image\ Dimensions.w \
	Chapter\ 6/Sound\ Durations.w \
	Headers/win32.h \
	$(NULL)

## See note in src/inweb/Makefile.am
dist-hook:
	export ABSDISTDIR=`readlink -f $(distdir)`; \
	$(MKDIR_P) --mode=u+w \
		$(distdir)/Chapter\ 1 \
		$(distdir)/Chapter\ 2 \
		$(distdir)/Chapter\ 3 \
		$(distdir)/Chapter\ 4 \
		$(distdir)/Chapter\ 5 \
		$(distdir)/Chapter\ 6 \
		$(distdir)/Figures \
		$(distdir)/Headers \
		$(distdir)/Woven \
		&& \
	cd $(srcdir) && \
	cp --preserve=mode,ownership,timestamps --parents \
		$(inlibsrc) "$$ABSDISTDIR"

GITIGNOREFILES = Woven

-include $(top_srcdir)/git.mk
