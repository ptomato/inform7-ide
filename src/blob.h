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

#define I7_TYPE_BLOB i7_blob_get_type()
G_DECLARE_FINAL_TYPE(I7Blob, i7_blob, I7, BLOB, GtkStack)

I7Blob *i7_blob_new(void);

void i7_blob_set_status(I7Blob *self, const char *status, bool show_stop_button);
void i7_blob_set_progress(I7Blob *self, double fraction);
void i7_blob_pulse_progress(I7Blob *self);
void i7_blob_clear_progress(I7Blob *self);

G_END_DECLS
