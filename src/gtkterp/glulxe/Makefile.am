bin_PROGRAMS = gtkterp-glulxe

gtkterp_glulxe_SOURCES = main.c files.c vm.c exec.c funcs.c operand.c \
	string.c glkop.c serial.c search.c gestalt.c osdepend.c unixstrt.c \
	gestalt.h glulxe.h opcodes.h

gtkterp_glulxe_CFLAGS = @GTKTERP_CFLAGS@ -I../garglk -DOS_UNIX
gtkterp_glulxe_LDFLAGS = -L/usr/lib -L/usr/local/lib
gtkterp_glulxe_LDADD = @GTKTERP_LIBS@ ../garglk/libgarglk.a

EXTRA_DIST = glulxdump.c
