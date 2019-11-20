SUBDIRS = inlib inweb cBlorb inform6 ni

# We build libinform7gui.a to link to the regular binary
# as well as the test binary
noinst_LIBRARIES = libosxcart.a libchimara.a libinform7gui.a

libosxcart_a_SOURCES = \
	osxcart/osxcart/init.c \
	osxcart/osxcart/init.h \
	osxcart/osxcart/plist.c \
	osxcart/osxcart/plist.h \
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
libosxcart_a_CFLAGS = @OSXCART_CFLAGS@

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
	chimara/libchimara/datetime.c \
	chimara/libchimara/dispatch.c \
	chimara/libchimara/event.c \
	chimara/libchimara/event.h \
	chimara/libchimara/fileref.c \
	chimara/libchimara/fileref.h \
	chimara/libchimara/garglk.c \
	chimara/libchimara/garglk.h \
	chimara/libchimara/gestalt.c \
	chimara/libchimara/gi_blorb.c \
	chimara/libchimara/gi_blorb.h \
	chimara/libchimara/gi_dispa.c \
	chimara/libchimara/gi_dispa.h \
	chimara/libchimara/glk.c \
	chimara/libchimara/glk.h \
	chimara/libchimara/glkstart.h \
	chimara/libchimara/glkunix.c \
	chimara/libchimara/glkunix.h \
	chimara/libchimara/graphics.c \
	chimara/libchimara/graphics.h \
	chimara/libchimara/hyperlink.c \
	chimara/libchimara/hyperlink.h \
	chimara/libchimara/init.c \
	chimara/libchimara/init.h \
	chimara/libchimara/input.c \
	chimara/libchimara/input.h \
	chimara/libchimara/magic.c \
	chimara/libchimara/magic.h \
	chimara/libchimara/mouse.c \
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
	chimara/libchimara/ui-buffer.c \
	chimara/libchimara/ui-buffer.h \
	chimara/libchimara/ui-graphics.c \
	chimara/libchimara/ui-graphics.h \
	chimara/libchimara/ui-grid.c \
	chimara/libchimara/ui-grid.h \
	chimara/libchimara/ui-message.c \
	chimara/libchimara/ui-message.h \
	chimara/libchimara/ui-misc.c \
	chimara/libchimara/ui-misc.h \
	chimara/libchimara/ui-style.c \
	chimara/libchimara/ui-style.h \
	chimara/libchimara/ui-textwin.c \
	chimara/libchimara/ui-textwin.h \
	chimara/libchimara/ui-window.c \
	chimara/libchimara/ui-window.h \
	chimara/libchimara/window.c \
	chimara/libchimara/window.h
libchimara_a_CPPFLAGS = \
	-DG_LOG_DOMAIN=\"Chimara\" \
	-DLOCALEDIR=\""$(datadir)/locale"\" \
	-DPLUGINDIR=\""$(pluginpath)"\" \
	-DPLUGINSOURCEDIR=\""$(abs_builddir)/../interpreters"\" \
	-I$(srcdir)/chimara
libchimara_a_CFLAGS = @CHIMARA_CFLAGS@ $(CHIMARA_EXTRAFLAGS)

BUILT_SOURCES = chimara-marshallers.c chimara-marshallers.h
chimara-marshallers.c: chimara/libchimara/chimara-marshallers.txt
	$(AM_V_GEN)glib-genmarshal --body --prefix=_chimara_marshal $< > $@

chimara-marshallers.h: chimara/libchimara/chimara-marshallers.txt
	$(AM_V_GEN)glib-genmarshal --header --prefix=_chimara_marshal $< > $@

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
frotz_la_CFLAGS = $(FROTZ_EXTRAFLAGS)
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
glulxe_la_LDFLAGS = -module $(PLUGIN_LIBTOOL_FLAGS)

glulxedocdir = $(datadir)/doc/$(PACKAGE)/glulxe
dist_glulxedoc_DATA = chimara/interpreters/glulxe/README

# Automatically generate version.h
GIT_MAJOR = 1
GIT_MINOR = 2
GIT_PATCH = 9
chimara/interpreters/git/version.h: Makefile.am
	$(AM_V_GEN)echo "// Automatically generated file -- do not edit!" > $@
	$(AM_V_at)echo "#define GIT_MAJOR" $(GIT_MAJOR) >> $@
	$(AM_V_at)echo "#define GIT_MINOR" $(GIT_MINOR) >> $@
	$(AM_V_at)echo "#define GIT_PATCH" $(GIT_PATCH) >> $@

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
git_la_CPPFLAGS = \
	-I$(top_srcdir)/src/chimara/libchimara \
	-I$(top_builddir)/src/chimara/interpreters/git \
	$(NULL)
git_la_CFLAGS = -DCHIMARA_EXTENSIONS -DUSE_INLINE $(GIT_EXTRAFLAGS)
git_la_LDFLAGS = -module $(PLUGIN_LIBTOOL_FLAGS)

gitdocdir = $(datadir)/doc/$(PACKAGE)/git
dist_gitdoc_DATA = chimara/interpreters/git/README.txt

CLEANFILES = chimara/interpreters/git/version.h

libinform7gui_a_SOURCES = \
	actions.c actions.h \
	app.c app.h \
	app-colorscheme.c \
	builder.c builder.h \
	configfile.c configfile.h \
	diffseq.h \
	document.c document.h \
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
	panel.c panel.h \
	placeholder-entry.c placeholder-entry.h \
	prefs.c prefs.h \
	searchwindow.c searchwindow.h \
	skein.c skein.h \
	skein-view.c skein-view.h \
	source-view.c source-view.h \
	spawn.c spawn.h \
	story.c story.h \
	story-source.c story-results.c story-index.c story-settings.c \
	story-compile.c story-game.c story-skein.c story-transcript.c \
	transcript-diff.c transcript-diff.h \
	transcript-renderer.c transcript-renderer.h \
	welcomedialog.c welcomedialog.h
libinform7gui_a_CPPFLAGS = \
	-DPACKAGE_DATA_DIR=\""$(datadir)"\" \
	-DPACKAGE_LIBEXEC_DIR=\""$(pkglibexecdir)"\" \
	-I$(srcdir)/osxcart \
	-I$(srcdir)/chimara
libinform7gui_a_CFLAGS = @INFORM7_CFLAGS@ $(WARNINGFLAGS)

bin_PROGRAMS = gnome-inform7
gnome_inform7_SOURCES = main.c
gnome_inform7_CPPFLAGS = -DPACKAGE_LOCALE_DIR=\""$(datadir)/locale"\"
gnome_inform7_CFLAGS = @INFORM7_CFLAGS@ $(WARNINGFLAGS)
gnome_inform7_LDADD = @INFORM7_LIBS@ @OSXCART_LIBS@ @CHIMARA_LIBS@ \
	$(INTLLIBS) \
	libosxcart.a \
	-lm
# The following mystical incantation is needed because the default behavior of
# ld is not to include object files with no referenced symbols in the final
# binary product. But the Glk programs, which are dynamically loaded, reference
# symbols from the Glk API in libchimara.a, and the GtkBuilder references
# symbols in libinform7gui.a, which are not referenced by the main program. This
# tells ld to link in all object files in libchimara.a and libinform7gui.a.
gnome_inform7_LDFLAGS = -Wl,--export-dynamic \
	-Wl,--whole-archive,libinform7gui.a,libchimara.a,--no-whole-archive
# The gnome-inform7 executable must also depend on libchimara.a; because of the
# trickery above, Automake doesn't realize that.
gnome_inform7_DEPENDENCIES = libosxcart.a libchimara.a libinform7gui.a

# Build the test suite as well, in the same way
check_PROGRAMS = test
test_SOURCES = tests/test.c \
	tests/app-test.c tests/app-test.h \
	tests/skein-test.c tests/skein-test.h \
	tests/story-test.c tests/story-test.h \
	$(NULL)
test_CPPFLAGS = \
	$(gnome_inform7_CPPFLAGS) \
	-I$(srcdir)/osxcart \
	-DTEST_DATA_DIR=\""$(srcdir)/tests/"\" \
	$(NULL)
test_CFLAGS = $(gnome_inform7_CFLAGS)
test_LDADD = $(gnome_inform7_LDADD)
test_LDFLAGS = $(gnome_inform7_LDFLAGS)
test_DEPENDENCIES = $(gnome_inform7_DEPENDENCIES)

TESTS = test
# Skip the /app tests, because they are too tightly coupled to the data files,
# and will fail during make distcheck -- o, the days of innocence!
# The /skein test requires the GSettings schema to be installed.
SKIP_PATHS = \
	/app/create \
	/app/files \
	/app/extensions/install-remove \
	/app/extensions/get-builtin \
	/app/extensions/get-version \
	/app/extensions/case-insensitive \
	/app/colorscheme/install-remove \
	/app/colorscheme/get-current \
	/skein/import \
	/story/materials-file \
	/story/old-materials-file \
	/story/renames-materials-file \
	$(NULL)
LOG_COMPILER = $(XVFB_RUN) gtester -k --verbose `echo $(SKIP_PATHS) | sed 's,\(^\|\s\+\)/, -s=/,g'`
EXTRA_DIST += \
	tests/commands.rec \
	tests/Hereafter.inform/Settings.plist \
	tests/Hereafter.inform/Skein.skein \
	tests/Hereafter.inform/Source/story.ni \
	tests/Hereafter.inform/notes.rtf \
	tests/test_color_scheme.xml \
	$(NULL)

-include $(top_srcdir)/git.mk
