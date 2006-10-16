## Automake template for Inform program.

bin_PROGRAMS = inform

SRCS = arrays.c asm.c bpatch.c chars.c directs.c errors.c expressc.c	 \
expressp.c files.c inform.c lexer.c linker.c memory.c objects.c states.c \
symbols.c syntax.c tables.c text.c veneer.c verbs.c

HDRS = header.h

INCPATH = ,$(DESTDIR)$(localincdir),$(DESTDIR)$(localmoddir),$(DESTDIR)$(includedir),$(DESTDIR)$(moduledir)
MODPATH = ,$(DESTDIR)$(localmoddir),$(DESTDIR)$(moduledir)

MODDEF = -DModule_Directory=\"$(MODPATH)\"
INCDEF = -DInclude_Directory=\"$(INCPATH)\"

INCLUDES = -D$(HOST) $(MODDEF) $(INCDEF)

inform_SOURCES = $(SRCS) $(HDRS)

install-exec-hook:
	mv $(DESTDIR)$(bindir)/inform $(DESTDIR)$(bindir)/inform-$(INFORM_VERSION)-inform7

uninstall-hook:
	- chmod u+w $(DESTDIR)$(bindir)
	rm -f $(DESTDIR)$(bindir)/inform-$(INFORM_VERSION)-inform7

@make_include@
