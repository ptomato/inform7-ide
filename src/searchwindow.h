/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 * SPDX-FileCopyrightText: 2006-2010 Philip Chimento <philip.chimento@gmail.com>
 */

#ifndef _SEARCHWINDOW_H
#define _SEARCHWINDOW_H

#include "config.h"

#include <glib.h>
#include <glib-object.h>
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

void i7_search_window_free_index(void);

#endif /* _SEARCHWINDOW_H */
