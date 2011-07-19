SUBDIRS = inweb cBlorb inform6 ni

AM_CFLAGS = -Wall

# This line won't be necessary anymore in Automake 1.10a
pkglibexecdir = $(libexecdir)/$(PACKAGE)

noinst_LIBRARIES = libosxcart.a libchimara.a

libosxcart_a_SOURCES = \
	osxcart/osxcart/init.c \
	osxcart/osxcart/init.h \
	osxcart/osxcart/plist.c \
	osxcart/osxcart/plist.h \
	osxcart/osxcart/plist-common.c \
	osxcart/osxcart/plist-common.h \
	osxcart/osxcart/plist-write.c \
	osxcart/osxcart/plist-parser.c \
	osxcart/osxcart/rtf.c \
	osxcart/osxcart/rtf.h \
	osxcart/osxcart/rtf-serialize.c \
	osxcart/osxcart/rtf-serialize.h \
	osxcart/osxcart/rtf-deserialize.c \
	osxcart/osxcart/rtf-deserialize.h \
	osxcart/osxcart/rtf-document.c \
	osxcart/osxcart/rtf-document.h \
	osxcart/osxcart/rtf-fonttbl.c \
	osxcart/osxcart/rtf-colortbl.c \
	osxcart/osxcart/rtf-ignore.c \
	osxcart/osxcart/rtf-ignore.h \
	osxcart/osxcart/rtf-footnote.c \
	osxcart/osxcart/rtf-picture.c \
	osxcart/osxcart/rtf-field.c \
	osxcart/osxcart/rtf-stylesheet.c \
	osxcart/osxcart/rtf-langcode.c \
	osxcart/osxcart/rtf-langcode.h \
	osxcart/osxcart/rtf-state.c \
	osxcart/osxcart/rtf-state.h
libosxcart_a_CPPFLAGS = \
	-DG_LOG_DOMAIN=\"Osxcart\" \
	-DLOCALEDIR=\""$(datadir)/locale"\" \
	-I$(srcdir)/osxcart
libosxcart_a_CFLAGS = @OSXCART_CFLAGS@ $(AM_CFLAGS)

pluginpath = $(pkglibdir)

libchimara_a_SOURCES = \
	chimara/libchimara/abort.c \
	chimara/libchimara/abort.h \
	chimara/libchimara/case.c \
	chimara/libchimara/charset.c \
	chimara/libchimara/charset.h \
	chimara/libchimara/chimara-glk.c \
	chimara/libchimara/chimara-glk.h \
	chimara/libchimara/chimara-glk-private.h \
	chimara/libchimara/chimara-if.c \
	chimara/libchimara/chimara-if.h \
	chimara-marshallers.c \
	chimara-marshallers.h \
	chimara/libchimara/dispatch.c \
	chimara/libchimara/event.c \
	chimara/libchimara/event.h \
	chimara/libchimara/fileref.c \
	chimara/libchimara/fileref.h \
	chimara/libchimara/garglk.c \
	chimara/libchimara/garglk.h \
	chimara/libchimara/gestalt.c \
	chimara/libchimara/graphics.c \
	chimara/libchimara/graphics.h \
	chimara/libchimara/gi_blorb.c \
	chimara/libchimara/gi_blorb.h \
	chimara/libchimara/gi_dispa.c \
	chimara/libchimara/gi_dispa.h \
	chimara/libchimara/glk.c \
	chimara/libchimara/glk.h \
	chimara/libchimara/glkstart.h \
	chimara/libchimara/glkunix.c \
	chimara/libchimara/glkunix.h \
	chimara/libchimara/hyperlink.c \
	chimara/libchimara/hyperlink.h \
	chimara/libchimara/init.c \
	chimara/libchimara/init.h \
	chimara/libchimara/input.c \
	chimara/libchimara/input.h \
	chimara/libchimara/magic.c \
	chimara/libchimara/magic.h \
	chimara/libchimara/mouse.c \
	chimara/libchimara/mouse.h \
	chimara/libchimara/pager.c \
	chimara/libchimara/pager.h \
	chimara/libchimara/resource.c \
	chimara/libchimara/resource.h \
	chimara/libchimara/schannel.c \
	chimara/libchimara/schannel.h \
	chimara/libchimara/stream.c \
	chimara/libchimara/stream.h \
	chimara/libchimara/strio.c \
	chimara/libchimara/strio.h \
	chimara/libchimara/style.c \
	chimara/libchimara/style.h \
	chimara/libchimara/timer.c \
	chimara/libchimara/timer.h \
	chimara/libchimara/window.c \
	chimara/libchimara/window.h
libchimara_a_CPPFLAGS = \
	-DG_LOG_DOMAIN=\"Chimara\" \
	-DLOCALEDIR=\""$(datadir)/locale"\" \
	-DPLUGINDIR=\""$(pluginpath)"\" \
	-DPLUGINSOURCEDIR=\""$(abs_builddir)/../interpreters"\" \
	-I$(srcdir)/chimara
libchimara_a_CFLAGS = @CHIMARA_CFLAGS@ $(AM_CFLAGS)

BUILT_SOURCES = chimara-marshallers.c chimara-marshallers.h
chimara-marshallers.c: chimara/libchimara/chimara-marshallers.txt
	glib-genmarshal --body --prefix=_chimara_marshal $< > $@

chimara-marshallers.h: chimara/libchimara/chimara-marshallers.txt
	glib-genmarshal --header --prefix=_chimara_marshal $< > $@
EXTRA_DIST = chimara/libchimara/chimara-marshallers.txt

pkglib_LTLIBRARIES = frotz.la glulxe.la git.la

frotz_la_SOURCES = \
	chimara/interpreters/frotz/buffer.c \
	chimara/interpreters/frotz/err.c \
	chimara/interpreters/frotz/fastmem.c \
	chimara/interpreters/frotz/files.c \
	chimara/interpreters/frotz/input.c \
	chimara/interpreters/frotz/main.c \
	chimara/interpreters/frotz/math.c \
	chimara/interpreters/frotz/object.c \
	chimara/interpreters/frotz/process.c \
	chimara/interpreters/frotz/quetzal.c \
	chimara/interpreters/frotz/random.c \
	chimara/interpreters/frotz/redirect.c \
	chimara/interpreters/frotz/sound.c \
	chimara/interpreters/frotz/stream.c \
	chimara/interpreters/frotz/table.c \
	chimara/interpreters/frotz/text.c \
	chimara/interpreters/frotz/variable.c \
	chimara/interpreters/frotz/glkscreen.c \
	chimara/interpreters/frotz/glkmisc.c \
	chimara/interpreters/frotz/frotz.h \
	chimara/interpreters/frotz/glkfrotz.h \
	chimara/interpreters/frotz/glkio.h
frotz_la_CPPFLAGS = \
	-I$(srcdir)/chimara/interpreters/frotz \
	-I$(srcdir)/chimara/libchimara \
	-I$(srcdir)/chimara
frotz_la_CFLAGS = -Wno-pointer-sign -Wno-implicit-function-declaration \
	-Wno-unused-variable -Wno-unused-but-set-variable $(AM_CFLAGS)
frotz_la_LDFLAGS = -module $(PLUGIN_LIBTOOL_FLAGS)

frotzdocdir = $(datadir)/doc/$(PACKAGE)/frotz
dist_frotzdoc_DATA = \
	chimara/interpreters/frotz/AUTHORS \
	chimara/interpreters/frotz/COPYING \
	chimara/interpreters/frotz/README \
	chimara/interpreters/frotz/TODO

glulxe_la_SOURCES = \
	chimara/interpreters/glulxe/accel.c \
	chimara/interpreters/glulxe/exec.c \
	chimara/interpreters/glulxe/files.c \
	chimara/interpreters/glulxe/float.c \
	chimara/interpreters/glulxe/funcs.c \
	chimara/interpreters/glulxe/gestalt.c \
	chimara/interpreters/glulxe/gestalt.h \
    chimara/interpreters/glulxe/glkop.c \
    chimara/interpreters/glulxe/glulxe.h \
    chimara/interpreters/glulxe/heap.c \
    chimara/interpreters/glulxe/main.c \
    chimara/interpreters/glulxe/opcodes.h \
    chimara/interpreters/glulxe/operand.c \
    chimara/interpreters/glulxe/osdepend.c \
    chimara/interpreters/glulxe/profile.c \
    chimara/interpreters/glulxe/search.c \
    chimara/interpreters/glulxe/serial.c \
    chimara/interpreters/glulxe/string.c \
    chimara/interpreters/glulxe/unixstrt.c \
    chimara/interpreters/glulxe/vm.c 
glulxe_la_CPPFLAGS = -I$(top_srcdir)/src/chimara/libchimara -DOS_UNIX
glulxe_la_CFLAGS = -Wall -Wmissing-prototypes -Wstrict-prototypes \
	-Wno-unused-value -Wno-strict-aliasing -Wno-uninitialized \
	-Wno-unused-variable -Wno-unused-function -Wno-unused-but-set-variable \
	$(AM_CFLAGS)
glulxe_la_LDFLAGS = -module $(PLUGIN_LIBTOOL_FLAGS)

glulxedocdir = $(datadir)/doc/$(PACKAGE)/glulxe
dist_glulxedoc_DATA = chimara/interpreters/glulxe/README

# Automatically generate version.h
GIT_MAJOR = 1
GIT_MINOR = 2
GIT_PATCH = 8
chimara/interpreters/git/version.h: Makefile.am
	echo "// Automatically generated file -- do not edit!" > $@
	echo "#define GIT_MAJOR" $(GIT_MAJOR) >> $@
	echo "#define GIT_MINOR" $(GIT_MINOR) >> $@
	echo "#define GIT_PATCH" $(GIT_PATCH) >> $@

BUILT_SOURCES += chimara/interpreters/git/version.h
git_la_SOURCES = \
	chimara/interpreters/git/version.h \
	chimara/interpreters/git/git.h \
	chimara/interpreters/git/config.h \
	chimara/interpreters/git/compiler.h \
	chimara/interpreters/git/memory.h \
	chimara/interpreters/git/opcodes.h \
    chimara/interpreters/git/labels.inc \
    chimara/interpreters/git/compiler.c \
    chimara/interpreters/git/gestalt.c \
    chimara/interpreters/git/git.c \
    chimara/interpreters/git/git_unix.c \
    chimara/interpreters/git/glkop.c \
    chimara/interpreters/git/heap.c \
    chimara/interpreters/git/memory.c \
    chimara/interpreters/git/opcodes.c \
    chimara/interpreters/git/operands.c \
    chimara/interpreters/git/peephole.c \
    chimara/interpreters/git/savefile.c \
    chimara/interpreters/git/saveundo.c \
    chimara/interpreters/git/search.c \
    chimara/interpreters/git/terp.c \
    chimara/interpreters/git/accel.c
git_la_CPPFLAGS = -I$(top_srcdir)/src/chimara/libchimara
git_la_CFLAGS = -DGARGLK -DUSE_INLINE -Wno-pointer-sign \
	-Wno-unused-but-set-variable -Wno-strict-aliasing \
	-Wno-implicit-function-declaration $(AM_CFLAGS)
git_la_LDFLAGS = -module $(PLUGIN_LIBTOOL_FLAGS)

gitdocdir = $(datadir)/doc/$(PACKAGE)/git
dist_gitdoc_DATA = chimara/interpreters/git/README.txt

CLEANFILES = chimara/interpreters/git/version.h

bin_PROGRAMS = gnome-inform7
gnome_inform7_SOURCES = \
	actions.c actions.h \
	app.c app.h app-private.h \
	builder.c builder.h \
	colorscheme.c colorscheme.h \
	configfile.c configfile.h \
	document.c document.h document-private.h \
	document-search.c \
	elastic.c elastic.h \
	error.c error.h \
	extension.c extension.h \
	file.c file.h \
	history.c history.h \
	html.c html.h \
	lang.c lang.h \
	newdialog.c newdialog.h \
	node.c node.h \
	notepad.c \
	panel.c panel.h panel-private.h \
	placeholder-entry.c placeholder-entry.h \
	prefs.c prefs.h \
	searchwindow.c searchwindow.h \
	skein.c skein.h \
	skein-view.c skein-view.h \
	source-view.c source-view.h \
	spawn.c spawn.h \
	story.c story.h story-private.h \
	story-source.c story-errors.c story-index.c story-settings.c \
	story-compile.c story-game.c story-skein.c \
	welcomedialog.c welcomedialog.h \
	main.c
gnome_inform7_CPPFLAGS = \
	-DPACKAGE_DATA_DIR=\""$(datadir)"\" \
	-DPACKAGE_LIBEXEC_DIR=\""$(pkglibexecdir)"\" \
	-DPACKAGE_LOCALE_DIR=\""$(prefix)/$(DATADIRNAME)/locale"\" \
	-I$(srcdir)/osxcart \
	-I$(srcdir)/chimara
gnome_inform7_CFLAGS = -g @INFORM7_CFLAGS@ $(AM_CFLAGS)
gnome_inform7_LDADD = \
	@INFORM7_LIBS@ @OSXCART_LIBS@ @CHIMARA_LIBS@ \
	$(INTLLIBS) \
	libosxcart.a \
	libchimara.a
# The following mystical incantation is needed because the Glk programs, which
# are dynamically loaded, reference symbols from the Glk API in libchimara.a
# which are not referenced by the main program. The default behavior of ld is
# not to include object files with no referenced symbols in the final binary
# product. This tells ld to link in all object files in libchimara.a.
gnome_inform7_LDFLAGS = -Wl,--export-dynamic \
	-Wl,--whole-archive,libchimara.a,--no-whole-archive

#noinst_PROGRAMS = skeintest
#skeintest_SOURCES = skeintest.c skein.c skein.h node.c node.h skein-view.c skein-view.h
#skeintest_CFLAGS = @INFORM7_CFLAGS@ $(AM_CFLAGS)
#skeintest_LDADD = @INFORM7_LIBS@

GITIGNOREFILES = $(noinst_LIBRARIES)

-include $(top_srcdir)/git.mk
