/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 * SPDX-FileCopyrightText: Philip Chimento <philip.chimento@gmail.com>
 */

#pragma once

#include "config.h"

#include <glib.h>
#include <glib-object.h>
#include <gtk/gtk.h>

#include "document.h"

#define I7_TYPE_SEARCH_WINDOW i7_search_window_get_type()
G_DECLARE_FINAL_TYPE(I7SearchWindow, i7_search_window, I7, SEARCH_WINDOW, GtkDialog)

typedef enum {
    I7_SEARCH_TARGET_SOURCE        = 0x01,
    I7_SEARCH_TARGET_EXTENSIONS    = 0x02,
    I7_SEARCH_TARGET_DOCUMENTATION = 0x04,
} I7SearchTarget;

I7SearchWindow *i7_search_window_new(I7Document *document);
void i7_search_window_prefill_ui(I7SearchWindow *self, const char *text, I7SearchTarget target, I7SearchFlags flags);
void i7_search_window_do_search(I7SearchWindow *self);

void i7_search_window_free_index(void);
