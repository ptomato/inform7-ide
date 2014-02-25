pkglibexec_PROGRAMS = \
	inform-@INFORM6_VERSION_MAJOR@.@INFORM6_VERSION_MINOR@-biplatform
inform_@INFORM6_VERSION_MAJOR@_@INFORM6_VERSION_MINOR@_biplatform_SOURCES = \
	arrays.c asm.c bpatch.c chars.c directs.c errors.c expressc.c expressp.c \
	files.c header.h inform.c lexer.c linker.c memory.c objects.c states.c \
	symbols.c syntax.c tables.c text.c veneer.c verbs.c
inform_@INFORM6_VERSION_MAJOR@_@INFORM6_VERSION_MINOR@_biplatform_CFLAGS = \
	-ansi -DLINUX -Wno-pointer-to-int-cast

inform6docdir = $(datadir)/doc/$(PACKAGE)/inform6
dist_inform6doc_DATA = readme.txt ReleaseNotes.html

-include $(top_srcdir)/git.mk
