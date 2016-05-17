mimedir = $(datadir)/mime/packages
mime = inform7.xml
dist_mime_DATA = $(mime)
EXTRA_DIST = $(mime).in

# Merge translations back into a new XML file
# GETTEXTDATADIR(S) is an undocumented hack to use uninstalled ITS rules
$(mime): $(mime).in
	$(AM_V_GEN)export GETTEXTDATADIR=$(top_srcdir)/data; \
	export GETTEXTDATADIRS=''; \
	$(MSGFMT) --xml --template $< -d $(top_srcdir)/po -o $@

CLEANFILES = $(mime)

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
