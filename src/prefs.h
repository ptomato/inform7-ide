/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 * SPDX-FileCopyrightText: Philip Chimento <philip.chimento@gmail.com>
 */

#ifndef PREFS_H
#define PREFS_H

#include "config.h"

#include <glib.h>
#include <glib-object.h>
#include <gtk/gtk.h>
#include <gtksourceview/gtksource.h>
#include <handy.h>

G_BEGIN_DECLS

#define I7_TYPE_PREFS_WINDOW i7_prefs_window_get_type()
G_DECLARE_FINAL_TYPE(I7PrefsWindow, i7_prefs_window, I7, PREFS_WINDOW, HdyPreferencesWindow)

I7PrefsWindow *i7_prefs_window_new(void);
void i7_prefs_window_bind_settings(I7PrefsWindow *self, GSettings *prefs);

gboolean update_style(GtkSourceBuffer *buffer);
gboolean update_tabs(GtkSourceView *view);

G_END_DECLS

#endif
