## Automake template for Inform libraries.

moduledir = $(installdir)/module
module_DATA = $(MODULES) $(INCLUDES)

MODULES = parserm.m5 verblibm.m5

INCLUDES = english.h grammar.h linklpa.h linklv.h parser.h parserm.h	\
verblib.h verblibm.h infix.h

EXTRA_NAMES = English.h Grammar.h Parser.h Verblib.h VerbLib.h

EXTRA_DIST = $(INCLUDES)

ETAGS_ARGS = $(ETAGS_INFORM) $(INCLUDES)

TAGS_DEPENDENCIES = $(INCLUDES)

SUFFIXES = .h .m5

INFORM = $(top_builddir)/src/inform

INFFLAGS = -x +include_path=$(srcdir)/ +module_path=$(srcdir)/	\
+language_name=english

.h.m5:;	$(INFORM) $(INFFLAGS) -M $< $@

if CYGWIN
install-data-hook: add-localdir
uninstall-local: remove-localdir
else
all-local: add-local-names
install-data-hook: add-localdir add-names
uninstall-local: remove-localdir remove-names
endif

add-localdir:
	$(mkinstalldirs) $(DESTDIR)$(localmoddir)

remove-localdir:
	rmdir $(DESTDIR)$(localmoddir)

add-local-names:
	@- cd $(top_builddir)/lib;					\
	for file in $(EXTRA_NAMES); do					\
	   realfile=`echo $$file | tr '[A-Z]' '[a-z]'`;			\
	   test -r $$file || $(LN_S) $(srcdir)/$$realfile $$file;	\
	done

add-names:
	@ cd $(DESTDIR)$(moduledir);					\
	for file in $(EXTRA_NAMES); do					\
	   realfile=`echo $$file | tr '[A-Z]' '[a-z]'`;			\
	   echo " linking $$file to $$realfile";			\
	   rm -f $$file;						\
	   $(LN_S) $$realfile $$file;					\
	done

remove-names:
	@ for file in $(EXTRA_NAMES); do				\
	   echo " unlinking $$file";					\
	   rm -f $(DESTDIR)$(moduledir)/$$file;				\
	done

CLEANFILES = $(MODULES) $(EXTRA_NAMES)

$(MODULES): $(INFORM)

@make_include@
