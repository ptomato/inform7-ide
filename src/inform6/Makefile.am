pkglibexec_PROGRAMS = inform6
inform6_SOURCES = \
	arrays.c asm.c bpatch.c chars.c directs.c errors.c expressc.c expressp.c \
	files.c header.h inform.c lexer.c linker.c memory.c objects.c states.c \
	symbols.c syntax.c tables.c text.c veneer.c verbs.c
inform6_CFLAGS = -ansi -DLINUX $(INFORM6_EXTRAFLAGS)

inform6docdir = $(datadir)/doc/$(PACKAGE)/inform6
dist_inform6doc_DATA = readme.txt licence.txt DebugFileFormat.txt \
    ReleaseNotes.html

-include $(top_srcdir)/git.mk
