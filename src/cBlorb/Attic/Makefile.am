## Process this file with automake to produce Makefile.in

INCLUDES = \
	-DPACKAGE_DATA_DIR=\""$(datadir)"\" \
	-DPACKAGE_LOCALE_DIR=\""$(prefix)/$(DATADIRNAME)/locale"\" \
	@PACKAGE_CFLAGS@

bin_PROGRAMS = cblorb

cblorb_SOURCES = cblorb.c

cblorb_LDADD = @PACKAGE_LIBS@ $(INTLLIBS)

cblorb_LDFLAGS =
