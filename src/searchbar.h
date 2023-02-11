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

#include "document.h"

G_BEGIN_DECLS

#define I7_TYPE_SEARCH_BAR i7_search_bar_get_type()
G_DECLARE_FINAL_TYPE(I7SearchBar, i7_search_bar, I7, SEARCH_BAR, GtkSearchBar)

I7SearchBar *i7_search_bar_new(I7Document *document);

void i7_search_bar_activate(I7SearchBar *self, bool replace_mode);
const char *i7_search_bar_get_search_string(I7SearchBar *self);
void i7_search_bar_set_target_description(I7SearchBar *self, const char *descr);
void i7_search_bar_set_not_found(I7SearchBar *self, bool not_found);
void i7_search_bar_set_can_replace(I7SearchBar *self, bool can_replace);
void i7_search_bar_set_can_restrict(I7SearchBar *self, bool can_restrict);

G_END_DECLS
