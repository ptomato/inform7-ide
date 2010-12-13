#ifndef STYLE_H
#define STYLE_H

#include <gtk/gtk.h>
#include <glib.h>
#include "glk.h"
#include "chimara-glk.h"

G_GNUC_INTERNAL void style_init_textbuffer(GtkTextBuffer *buffer);
G_GNUC_INTERNAL void style_init_textgrid(GtkTextBuffer *buffer);
G_GNUC_INTERNAL void style_init_more_prompt(winid_t win);
G_GNUC_INTERNAL void style_init(ChimaraGlk *glk);
G_GNUC_INTERNAL void style_update(ChimaraGlk *glk);
G_GNUC_INTERNAL const gchar** style_get_tag_names();
G_GNUC_INTERNAL void reset_default_styles(ChimaraGlk *glk);
/*G_GNUC_INTERNAL void copy_default_styles_to_current_styles(ChimaraGlk *glk);*/
G_GNUC_INTERNAL GScanner *create_css_file_scanner(void);
G_GNUC_INTERNAL void scan_css_file(GScanner *scanner, ChimaraGlk *glk);
G_GNUC_INTERNAL PangoFontDescription *get_current_font(guint32 wintype);
G_GNUC_INTERNAL GtkTextTag* gtk_text_tag_copy(GtkTextTag *tag);
G_GNUC_INTERNAL void glkcolor_to_gdkcolor(glui32 val, GdkColor *color);

typedef struct StyleSet {
	GHashTable *text_grid;
	GHashTable *text_buffer;
} StyleSet;

#define CHIMARA_NUM_STYLES 13

#endif
