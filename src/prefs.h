/* Copyright (C) 2006-2009, 2010, 2011, 2013, 2018 P. F. Chimento
 * This file is part of GNOME Inform 7.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef PREFS_H
#define PREFS_H

#include <glib-object.h>
#include <glib.h>
#include <gtk/gtk.h>
#include <gtksourceview/gtksource.h>

typedef struct {
	/* Global pointer to preferences window */
	GtkWidget *window;
	/* Global pointers to various widgets */
	GtkWidget *prefs_notebook;
	GtkTreeView *schemes_view;
	GtkWidget *style_remove;
	GtkSourceView *tab_example;
	GtkSourceView *source_example;
	GtkTreeView *extensions_view;
	GtkWidget *extensions_add;
	GtkWidget *extensions_remove;
	GtkWidget *auto_number;
	GtkWidget *clean_index_files;
	GtkListStore *schemes_list;
} I7PrefsWidgets;

I7PrefsWidgets *create_prefs_window(GSettings *prefs, GtkBuilder *builder);
void populate_schemes_list(GtkListStore *list);

/* The tabs in the preferences window */
typedef enum {
	I7_PREFS_AUTHOR,
	I7_PREFS_EDITING,
	I7_PREFS_FORMATTING,
	I7_PREFS_EXTENSIONS,
	I7_PREFS_ADVANCED
} I7PrefsTabs;

gboolean update_style(GtkSourceBuffer *buffer);
gboolean update_font(GtkWidget *widget);
gboolean update_tabs(GtkSourceView *view);
void select_style_scheme(GtkTreeView *view, const gchar *id);

#endif
