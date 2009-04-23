# This line won't be necessary anymore in Automake 1.10a
pkglibexecdir = $(libexecdir)/$(PACKAGE)

pkglibexec_PROGRAMS = gtkterp-git
gtkterp_git_SOURCES = accel.c compiler.c compiler.h config.h gestalt.c git.c \
	git.h git_unix.c glkop.c heap.c memory.c memory.h opcodes.c opcodes.h \
	operands.c peephole.c savefile.c saveundo.c search.c terp.c version.h
gtkterp_git_CFLAGS = @GTKTERP_CFLAGS@ -I../garglk -DUSE_INLINE -DUSE_DIRECT_THREADING
gtkterp_git_LDFLAGS = -L/usr/lib -L/usr/local/lib
gtkterp_git_LDADD = @GTKTERP_LIBS@ ../garglk/libgarglk.a

gitdocdir = $(datadir)/doc/$(PACKAGE)/git
dist_gitdoc_DATA = README.txt

EXTRA_DIST = labels.inc
