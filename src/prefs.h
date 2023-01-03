/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 * SPDX-FileCopyrightText: 2006-2011, 2013 Philip Chimento <philip.chimento@gmail.com>
 */

#ifndef PREFS_H
#define PREFS_H

#include "config.h"

#include <glib.h>
#include <glib-object.h>
#include <gtk/gtk.h>
#include <gtksourceview/gtksource.h>

G_BEGIN_DECLS

struct _I7PrefsWindow {
	GtkDialog parent;

	/* Public pointers to various widgets */
	GtkEntry *author_name;
	GtkCheckButton *auto_indent;
	GtkCheckButton *clean_build_files;
	GtkFontButton *custom_font;
	GtkCheckButton *enable_highlighting;
	GtkComboBoxText *font_set;
	GtkComboBoxText *font_size;
	GtkComboBoxText *glulx_combo;
	GtkWidget *prefs_notebook;
	GtkTreeView *schemes_view;
	GtkCheckButton *show_debug_tabs;
	GtkWidget *style_remove;
	GtkSourceView *tab_example;
	GtkScale *tab_ruler;
	GtkSourceView *source_example;
	GtkWidget *auto_number;
	GtkWidget *clean_index_files;
	GtkListStore *schemes_list;
};

#define I7_TYPE_PREFS_WINDOW i7_prefs_window_get_type()
G_DECLARE_FINAL_TYPE(I7PrefsWindow, i7_prefs_window, I7, PREFS_WINDOW, GtkDialog)

I7PrefsWindow *i7_prefs_window_new(void);

void init_prefs_window(I7PrefsWindow *self, GSettings *prefs);
void populate_schemes_list(GtkListStore *list);

gboolean update_style(GtkSourceBuffer *buffer);
gboolean update_tabs(GtkSourceView *view);
void select_style_scheme(GtkTreeView *view, const gchar *id);

G_END_DECLS

#endif
