bin_PROGRAMS = gtkterp-git

gtkterp_git_SOURCES = git.c memory.c compiler.c opcodes.c operands.c \
	peephole.c terp.c glkop.c search.c git_unix.c savefile.c saveundo.c \
	gestalt.c heap.c

gtkterp_git_CFLAGS = @GTKTERP_CFLAGS@ -I../garglk -DUSE_INLINE -DUSE_DIRECT_THREADING
gtkterp_git_LDFLAGS = -L/usr/lib -L/usr/local/lib
gtkterp_git_LDADD = @GTKTERP_LIBS@ ../garglk/libgarglk.a

