iconsdir = $(datadir)/icons/hicolor

# Named Icons
# -----------
# com.inform7.IDE.png: named application icon (inform.iconset)
# inform7-builtin.png: named icon for built-in extension
# inform7-materials.png: named emblem for materials folder
#
# MIME types
# ----------
# application/x-inform: Inform 7 project (informfile.iconset)
# application/x-inform-materials: Materials folder (materialsfile.iconset)
# application/x-inform-skein+xml: Skein file (skeinfile.iconset)
# text/x-inform: Inform 6 source code (i6file.iconset, inffile.iconset)
# text/x-natural-inform: Inform 7 source code (i7file.iconset, nifile.iconset)
# text/x-natural-inform-extension: Extension file (i7xfile.iconset)
# text/x-blurb: Blurb file (blurbfile.iconset)

nobase_dist_icons_DATA = \
	16x16/apps/com.inform7.IDE.png \
	32x32/apps/com.inform7.IDE.png \
	48x48/apps/com.inform7.IDE.png \
	64x64/apps/com.inform7.IDE.png \
	128x128/apps/com.inform7.IDE.png \
	256x256/apps/com.inform7.IDE.png \
	16x16/actions/inform7-builtin.png \
	32x32/actions/inform7-builtin.png \
	48x48/actions/inform7-builtin.png \
	128x128/actions/inform7-builtin.png \
	16x16/emblems/inform7-materials.png \
	32x32/emblems/inform7-materials.png \
	48x48/emblems/inform7-materials.png \
	64x64/emblems/inform7-materials.png \
	128x128/emblems/inform7-materials.png \
	256x256/emblems/inform7-materials.png \
	16x16/mimetypes/application-x-inform.png \
	32x32/mimetypes/application-x-inform.png \
	48x48/mimetypes/application-x-inform.png \
	64x64/mimetypes/application-x-inform.png \
	128x128/mimetypes/application-x-inform.png \
	256x256/mimetypes/application-x-inform.png \
	16x16/mimetypes/application-x-inform-materials.png \
	32x32/mimetypes/application-x-inform-materials.png \
	48x48/mimetypes/application-x-inform-materials.png \
	64x64/mimetypes/application-x-inform-materials.png \
	128x128/mimetypes/application-x-inform-materials.png \
	256x256/mimetypes/application-x-inform-materials.png \
	16x16/mimetypes/application-x-inform-skein+xml.png \
	32x32/mimetypes/application-x-inform-skein+xml.png \
	48x48/mimetypes/application-x-inform-skein+xml.png \
	64x64/mimetypes/application-x-inform-skein+xml.png \
	128x128/mimetypes/application-x-inform-skein+xml.png \
	256x256/mimetypes/application-x-inform-skein+xml.png \
	16x16/mimetypes/text-x-inform.png \
	32x32/mimetypes/text-x-inform.png \
	48x48/mimetypes/text-x-inform.png \
	64x64/mimetypes/text-x-inform.png \
	128x128/mimetypes/text-x-inform.png \
	256x256/mimetypes/text-x-inform.png \
	16x16/mimetypes/text-x-natural-inform.png \
	32x32/mimetypes/text-x-natural-inform.png \
	48x48/mimetypes/text-x-natural-inform.png \
	64x64/mimetypes/text-x-natural-inform.png \
	128x128/mimetypes/text-x-natural-inform.png \
	256x256/mimetypes/text-x-natural-inform.png \
	16x16/mimetypes/text-x-natural-inform-extension.png \
	32x32/mimetypes/text-x-natural-inform-extension.png \
	48x48/mimetypes/text-x-natural-inform-extension.png \
	64x64/mimetypes/text-x-natural-inform-extension.png \
	128x128/mimetypes/text-x-natural-inform-extension.png \
	256x256/mimetypes/text-x-natural-inform-extension.png \
	16x16/mimetypes/text-x-blurb.png \
	32x32/mimetypes/text-x-blurb.png \
	48x48/mimetypes/text-x-blurb.png \
	64x64/mimetypes/text-x-blurb.png \
	128x128/mimetypes/text-x-blurb.png \
	256x256/mimetypes/text-x-blurb.png \
	$(NULL)

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
.PHONY: update-icon-cache

-include $(top_srcdir)/git.mk
