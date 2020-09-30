stylesdir = $(pkgdatadir)/styles
dist_styles_DATA = \
	inform.xml \
	monochrome.xml \
	plain.xml \
	psychedelic.xml \
	subdued.xml \
	subtle.xml
dist_noinst_DATA = styles.pot
EXTRA_DIST = styles.its
MAINTAINERCLEANFILES = styles.pot

styles.pot: $(dist_styles_DATA) styles.its
	$(ITSTOOL) -i $(srcdir)/styles.its -o $@ $(addprefix $(srcdir)/,$(dist_styles_DATA))

-include $(top_srcdir)/git.mk
