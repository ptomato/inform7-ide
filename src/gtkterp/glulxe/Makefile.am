# This line won't be necessary anymore in Automake 1.10a
pkglibexecdir = $(libexecdir)/$(PACKAGE)

pkglibexec_PROGRAMS = gtkterp-glulxe
gtkterp_glulxe_SOURCES = main.c files.c vm.c exec.c funcs.c operand.c \
	string.c glkop.c serial.c search.c gestalt.c osdepend.c unixstrt.c \
	heap.c gestalt.h glulxe.h opcodes.h accel.c profile.c 
gtkterp_glulxe_CFLAGS = @GTKTERP_CFLAGS@ -I../garglk -DOS_UNIX
gtkterp_glulxe_LDFLAGS = -L/usr/lib -L/usr/local/lib
gtkterp_glulxe_LDADD = @GTKTERP_LIBS@ ../garglk/libgarglk.a

glulxedocdir = $(datadir)/doc/$(PACKAGE)/glulxe
dist_glulxedoc_DATA = README

EXTRA_DIST = glulxdump.c profile-analyze.py
