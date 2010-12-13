/* This file is part of GNOME Inform 7.
 * Copyright (c) 2006-2009 P. F. Chimento <philip.chimento@gmail.com>
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
 
#ifndef _SEARCHWINDOW_H
#define _SEARCHWINDOW_H

#include <glib-object.h>
#include <glib.h>
#include <gtk/gtk.h>
#include "document.h"

#define I7_TYPE_SEARCH_WINDOW            (i7_search_window_get_type())
#define I7_SEARCH_WINDOW(obj)            (G_TYPE_CHECK_INSTANCE_CAST((obj), I7_TYPE_SEARCH_WINDOW, I7SearchWindow))
#define I7_SEARCH_WINDOW_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST((klass), I7_TYPE_SEARCH_WINDOW, I7SearchWindowClass))
#define I7_IS_SEARCH_WINDOW(obj)         (G_TYPE_CHECK_INSTANCE_TYPE((obj), I7_TYPE_SEARCH_WINDOW))
#define I7_IS_SEARCH_WINDOW_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), I7_TYPE_SEARCH_WINDOW))
#define I7_SEARCH_WINDOW_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS((obj), I7_TYPE_SEARCH_WINDOW, I7SearchWindowClass))

typedef struct {
	GtkWindowClass parent_class;
} I7SearchWindowClass;

typedef struct {
	GtkWindow parent_instance;

	GtkWidget *search_text;
	GtkWidget *spinner;
	GtkWidget *results_view;
} I7SearchWindow;

GType i7_search_window_get_type(void) G_GNUC_CONST;
GtkWidget *i7_search_window_new(I7Document *document, const gchar *text, gboolean ignore_case, I7SearchType algorithm);
void i7_search_window_search_documentation(I7SearchWindow *self);
void i7_search_window_search_project(I7SearchWindow *self);
void i7_search_window_search_extensions(I7SearchWindow *self);
void i7_search_window_done_searching(I7SearchWindow *self);

#endif /* _SEARCHWINDOW_H */
