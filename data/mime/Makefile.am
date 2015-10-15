mimedir = $(datadir)/mime/packages
dist_mime_DATA = inform7.xml
dist_noinst_DATA = mime.pot inform7.xml.in

mime.pot: inform7.xml.in
	$(AM_V_GEN)$(ITSTOOL) -o $@ $<

# Merge translations back into a new XML file
inform7.xml: inform7.xml.in
	$(AM_V_GEN)$(ITSTOOL) -j $< -o $@ $(top_srcdir)/po/*.gmo

MAINTAINERCLEANFILES = $(dist_mime_DATA) mime.pot

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
