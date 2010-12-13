iconsdir = $(datadir)/icons/hicolor

nobase_dist_icons_DATA = \
	16x16/apps/inform7.png \
	32x32/apps/inform7.png \
	48x48/apps/inform7.png \
	128x128/apps/inform7.png  \
	16x16/actions/inform7-builtin.png \
	32x32/actions/inform7-builtin.png \
	48x48/actions/inform7-builtin.png \
	128x128/actions/inform7-builtin.png \
	16x16/mimetypes/application-x-inform.png \
	32x32/mimetypes/application-x-inform.png \
	48x48/mimetypes/application-x-inform.png \
	128x128/mimetypes/application-x-inform.png \
	16x16/mimetypes/text-x-inform.png \
	32x32/mimetypes/text-x-inform.png \
	48x48/mimetypes/text-x-inform.png \
	128x128/mimetypes/text-x-inform.png \
	16x16/mimetypes/text-x-natural-inform.png \
	32x32/mimetypes/text-x-natural-inform.png \
	48x48/mimetypes/text-x-natural-inform.png \
	128x128/mimetypes/text-x-natural-inform.png

# From Rhythmbox's icon install mechanism
gtk_update_icon_cache = gtk-update-icon-cache -f -t $(iconsdir)

install-data-hook: update-icon-cache
uninstall-hook: update-icon-cache
update-icon-cache:
	@-if test -z "$(DESTDIR)"; then \
		echo "Updating GTK icon cache."; \
		$(gtk_update_icon_cache); \
	else \
		echo "*** Icon cache not updated.  After (un)install, run this:"; \
		echo "***   $(gtk_update_icon_cache)"; \
	fi
