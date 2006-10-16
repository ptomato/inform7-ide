## Master automake template for Inform.

AUTOMAKE_OPTIONS = check-news

SUBDIRS = src lib include

EXTRA_DIST = AUTHORS VERSION

EXTRA_DIRS = config

dist-hook:
	cd $(srcdir)/tutor; $(MAKE) clean
	@ for dir in $(EXTRA_DIRS); do					\
	   echo Adding $$dir directory to distribution;			\
	   cp -rfp $(srcdir)/$$dir $(distdir);				\
	   rm -rf $(distdir)/$$dir/CVS;					\
	   rm -rf $(distdir)/$$dir/.cvsignore;				\
	done

@make_include@
