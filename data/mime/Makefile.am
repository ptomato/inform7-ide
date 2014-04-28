mimedir = $(datadir)/mime/packages
mime_DATA = inform7.xml

@INTLTOOL_XML_RULE@

CLEANFILES = $(mime_DATA)
EXTRA_DIST = inform7.xml.in

update_mime_database = update-mime-database $(DESTDIR)$(datadir)/mime

# Rule adapted from Bijiben and Brasero's MIME makefiles
install-data-hook: update-mime-database
uninstall-hook: update-mime-database
update-mime-database:
	@-if test -f $(DESTDIR)$(mimedir)/freedesktop.org.xml \
	&& which update-mime-database > /dev/null 2>&1; then \
		echo "Updating MIME database."; \
		$(update_mime_database); \
	else \
		echo "*** MIME database not updated.  After (un)install, run this:"; \
		echo "***   $(update_mime_database)"; \
	fi
.PHONY: update-mime-database

-include $(top_srcdir)/git.mk
