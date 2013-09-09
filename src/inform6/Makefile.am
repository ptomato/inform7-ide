pkglibexec_PROGRAMS = inform-6.32-biplatform
inform_6_32_biplatform_SOURCES = arrays.c asm.c bpatch.c chars.c directs.c \
	errors.c expressc.c expressp.c files.c header.h inform.c lexer.c linker.c \
	memory.c objects.c states.c symbols.c syntax.c tables.c text.c veneer.c \
	verbs.c
inform_6_32_biplatform_CFLAGS = -ansi -DLINUX -Wno-pointer-to-int-cast

inform6docdir = $(datadir)/doc/$(PACKAGE)/inform6
dist_inform6doc_DATA = readme.txt ReleaseNotes.html

-include $(top_srcdir)/git.mk
