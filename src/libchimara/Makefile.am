## Process this file with automake to produce Makefile.in

AM_CFLAGS = -Wall

noinst_LIBRARIES = libchimara.a

pluginpath = $(pkglibdir)

libchimara_a_SOURCES = \
	abort.c abort.h \
	case.c \
	charset.c charset.h \
	chimara-glk.c chimara-glk.h chimara-glk-private.h \
	chimara-if.c chimara-if.h \
	chimara-marshallers.c chimara-marshallers.h \
	dispatch.c \
	event.c event.h \
	fileref.c fileref.h \
	garglk.c garglk.h \
	gestalt.c \
	graphics.c graphics.h \
	gi_blorb.c gi_blorb.h \
	gi_dispa.c gi_dispa.h \
	glk.c glk.h \
	glkstart.h \
	glkunix.c glkunix.h \
	hyperlink.c hyperlink.h \
	init.c init.h \
	input.c input.h \
	magic.c magic.h \
	mouse.c mouse.h \
	pager.c pager.h \
	resource.c resource.h \
	schannel.c schannel.h \
	stream.c stream.h \
	strio.c strio.h \
	style.c style.h \
	timer.c timer.h \
	window.c window.h
libchimara_a_CPPFLAGS = \
	-DG_LOG_DOMAIN=\"Chimara\" \
	-DLOCALEDIR=\""$(datadir)/locale"\" \
	-DPLUGINDIR=\""$(pluginpath)"\" \
	-DPLUGINSOURCEDIR=\""$(abs_builddir)/../interpreters"\" \
	-I..
libchimara_a_CFLAGS = @CHIMARA_CFLAGS@ $(AM_CFLAGS)

BUILT_SOURCES = chimara-marshallers.c chimara-marshallers.h
chimara-marshallers.c: chimara-marshallers.txt
	glib-genmarshal --body --prefix=_chimara_marshal $< > $@

chimara-marshallers.h: chimara-marshallers.txt
	glib-genmarshal --header --prefix=_chimara_marshal $< > $@

EXTRA_DIST = chimara-marshallers.txt
