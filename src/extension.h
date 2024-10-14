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

#include "app.h"
#include "document.h"
#include "source-view.h"

#define I7_TYPE_EXTENSION            (i7_extension_get_type())
#define I7_EXTENSION(obj)            (G_TYPE_CHECK_INSTANCE_CAST((obj), I7_TYPE_EXTENSION, I7Extension))
#define I7_EXTENSION_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST((klass), I7_TYPE_EXTENSION, I7ExtensionClass))
#define I7_IS_EXTENSION(obj)         (G_TYPE_CHECK_INSTANCE_TYPE((obj), I7_TYPE_EXTENSION))
#define I7_IS_EXTENSION_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), I7_TYPE_EXTENSION))
#define I7_EXTENSION_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS((obj), I7_TYPE_EXTENSION, I7ExtensionClass))

typedef struct {
	I7DocumentClass parent_class;
} I7ExtensionClass;

typedef struct {
	I7Document parent_instance;

	I7SourceView *sourceview;
	GtkCssProvider *background_provider;
} I7Extension;

GType i7_extension_get_type(void) G_GNUC_CONST;
I7Extension *i7_extension_new(I7App *app, GFile *file, const char *title, const char *author);
I7Extension *i7_extension_new_from_file(I7App *app, GFile *file, gboolean readonly);
bool i7_extension_open(I7Extension *self, bool readonly);
void i7_extension_set_read_only(I7Extension *self, gboolean readonly);
