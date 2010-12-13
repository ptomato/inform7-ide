# This line won't be necessary anymore in Automake 1.10a
pkglibexecdir = $(libexecdir)/$(PACKAGE)

pkglibexec_PROGRAMS = gtkterp-frotz
gtkterp_frotz_SOURCES = buffer.c err.c fastmem.c files.c input.c main.c \
	math.c object.c process.c quetzal.c random.c redirect.c sound.c \
	stream.c table.c text.c variable.c glkscreen.c glkmisc.c frotz.h \
	glkfrotz.h glkio.h setup.h
gtkterp_frotz_CFLAGS = @GTKTERP_CFLAGS@ -I../garglk
gtkterp_frotz_LDFLAGS = -L/usr/lib -L/usr/local/lib
gtkterp_frotz_LDADD = @GTKTERP_LIBS@ ../garglk/libgarglk.a

frotzdocdir = $(datadir)/doc/$(PACKAGE)/frotz
dist_frotzdoc_DATA = AUTHORS COPYING README TODO

