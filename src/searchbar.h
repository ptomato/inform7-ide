/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 * SPDX-FileCopyrightText: Philip Chimento <philip.chimento@gmail.com>
 */

#pragma once

#include "config.h"

#include <stdbool.h>

#include <glib.h>
#include <glib-object.h>
#include <gtk/gtk.h>

G_BEGIN_DECLS

#define I7_TYPE_SEARCH_BAR i7_search_bar_get_type()
G_DECLARE_FINAL_TYPE(I7SearchBar, i7_search_bar, I7, SEARCH_BAR, GtkSearchBar)

I7SearchBar *i7_search_bar_new(void);

void i7_search_bar_activate(I7SearchBar *self, bool replace_mode, bool can_restrict, GtkWidget *view, const char *descr);

G_END_DECLS
